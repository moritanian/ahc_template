---
name: beam-search
description: AHC 向け edge beam search (lib/edge_beam.cpp) の実装ガイド。
  ライブラリの使い方、ビーム幅/ハッシュ dedup/多点再出発/枝刈り等の汎用
  テクニック、可変長 undo 用 Arena Pool パターンや commit API、動的 beam_width
  などの実装パターンを 1 本にまとめたリファレンス。ahc063 で得た learnings
  を取り込み済み。ビームサーチを書き始めるとき、チューニングで詰まったとき、
  undo/pool 周りで SIGSEGV/SIGABRT が出たときに参照する。
---

# Beam Search 実装ガイド (AHC)

`lib/edge_beam.cpp` を使った差分更新型 beam search (rhoo 方式 / post-order
Action 配列) の実装ガイド。Hash あり版 = `EdgeBeamSearch`、Hash なし版 =
`EdgeBeamSearchNoHash`。汎用の tips と、ahc063 で蓄積した実戦的な learnings を
まとめる。

---

## 1. lib/edge_beam.cpp の使い方

### StateConcept が要求する API

```cpp
struct State {
    // state を初期状態にセットする副作用のために呼ばれる。
    // 戻り値は lib 内で使われないが、StateConcept を満たすため返す必要がある。
    // Hash 版: pair<Cost, Hash>、NoHash 版: Cost を返す。
    pair<Cost, Hash> make_initial_node();  // Hash 版
    Cost             make_initial_node();  // NoHash 版
    // 指定 parent から可能な遷移を列挙し selector.push() で渡す
    void expand(int parent, Selector& selector);
    // 1 手進める (undo 情報を積む)
    void move_forward(Action action);
    // 1 手戻す (undo 情報を消費)
    void move_backward(Action action);
};
```

`expand` が生成する candidate 側の finished フラグを立てれば、
「実行可能解に到達したら即終了」モードが使える。
詳細は `return_finished_immediately` 項を参照。

### Config の主なフィールド

| フィールド | 意味 | 備考 |
|---|---|---|
| `max_turn` | 最大ターン数 | |
| `beam_width` | 初期 beam_width | |
| `max_beam_width` | 内部配列の確保サイズ (0 なら beam_width と同じ、動的調整するときは最大値を入れる) | |
| `tour_capacity` | post-order Action 配列のバッファ (≒ `beam_width * max_turn` 程度) | 超えると vector が倍増戦略でリアロケート (O(N) コピーが数回) |
| `hash_map_capacity` | ハッシュマップ容量 (`beam_width * 32 + 1` 程度が目安) | Hash 版のみ |
| `return_finished_immediately` | true にすると finished=true の candidate が 1 つ出た時点で即 return | |
| `is_time_over` | `function<bool()>`。毎ターン先頭で呼ばれ、true なら途中 best を返して打ち切り | |
| `get_beam_width` | `function<size_t(int turn)>`。毎ターン末尾で呼ばれ、次ターンの bw を返す (動的調整用) | |

### `return_finished_immediately` の使い分け

- **true** にする場面: ビーム内 turn と問題 turn が 1:1 対応で、かつ
  **ターン数最小化型** (最短経路・最短手数) の問題。最初に finished に
  到達したものが最適。
- **false** にする場面: ビーム内 turn と問題 turn が一致しない
  (例: 1 expand で複数手進む / 無効手で進まない)、または
  **コスト最小化型**で複数の finished 候補の中で cost を比較したい場合。
  `best_ret` / `best_cost` で最良のみ保持する。

### 典型的な呼び出し

```cpp
using BS = EdgeBeamSearch<Hash, Action, Cost, State>;
BS bs;
BS::Config cfg;
cfg.max_turn = M * 10;
cfg.beam_width = 200;
cfg.max_beam_width = 2000;        // 動的増加の上限
cfg.tour_capacity = cfg.beam_width * 100;
cfg.hash_map_capacity = cfg.beam_width * 32 + 1;
cfg.return_finished_immediately = true;
cfg.is_time_over = [&]{ return tk.elapsed() > limit_ms; };
cfg.get_beam_width = [&](int turn){ /* ... */ return size_t(new_bw); };

State state(input);
vector<Action> path = bs.beam_search(cfg, state);
```

---

## 2. 汎用テクニック (beam search 一般)

### 2.1 ハッシュによる類似状態の除去

Selector は **同じ hash のうち cost が良い方だけ残す** (dedup 機構組込済)。
効かせるコツ:

- **hash 関数は「進行方向として意味のある状態要素」だけ含める**。
  進行に無関係な要素 (例: 履歴全体) を含めると dedup が効かず beam が
  事実上広がる。逆に粗すぎると **本来違う状態を同一視して枝刈り過多**。
- ahc063 では `(頭座標, 頭の向き, cp, 残食料のハッシュ)` 程度に抑えると
  効果が出やすい。蛇全体の座標列をそのまま hash に入れると dedup が
  ほぼ効かない。
- ハッシュ衝突による誤除去も起きる。**Zobrist** や **mixing の良い 64bit**
  を使い、2^32 エントリでは衝突が目立つため **64bit のまま保持**。

### 2.2 ビーム幅の決め方

- **小さい bw で始め、失敗時に拡大**が基本。最初から最大値で走ると無駄。
- 成功したら **据え置き**。ノイズ seed を変えた multi-start で広げる。
- 失敗時の倍率は **1.1〜1.2**。それ以上だと粗すぎ、未満だと収束遅い。
- **問題規模 (N, M など) ごとに初期 bw を変える**。ahc063 では
  `N<13` は小 bw + 拡大、`N>=13` は回帰モデルから推定した
  `safe_initial_bw` で一発勝負、と分岐。
- Optuna などでパラメータ空間を探索して N ごとにチューニングする価値あり。
- **動的 bw** (`get_beam_width` コールバック) で「序盤広く終盤狭く」など
  の制御も可能 (reverse も効く場面あり)。

### 2.3 Multi-start (ノイズ再出発)

`expand` 内のコスト計算末尾に `rng() & mask` を加算。mask=3〜7 程度。
時間が余ればイテレーションを重ねて異なる解を試す。

- mask が大きすぎると評価順がシャッフルされ探索が拡散
- mask=0 にすると完全に同一の beam になり再走の意味がない
- イテレーションごとに seed を変え、前回解を覚えて best を取る

### 2.4 max_turn cutoff & depth pruning & stagnation cutoff

- **max_turn cutoff**: 最初に実行可能解 (`best_ok_turns`) が見つかったら
  次イテレーションからは `max_turn = best_ok_turns - 1` にしてタイ解を
  除外 (**strict improvement only**)。
- **depth pruning**: `turn + (要達成量 - 現在量) >= max_turn` のノードは
  時間内に間に合わないので `is_time_over` or 個別に skip。
- **stagnation cutoff**: ある進捗指標 (ahc063 では cp) が N 手以上改善
  しなければそのイテレーションを打ち切り、次 seed へ。

### 2.5 評価関数の設計原則

- **主項を他項より十分大きな係数で**。ahc063 は `cp` を 10000 倍して
  優先順位を絶対化し、同 cp 内で他項が比較される構造。
- **前提条件付き項** (dirty 時のみ bite 誘導ペナルティ、clean 時のみ
  lookahead など) を明確に分けて競合を避ける。
- **Lookahead は段数を増やすほど評価は正確になるが、hash 多様性を
  食って beam の多様性が落ちる**。ahc063 では 5 段で頭打ち。
- **偶数/奇数ターンで重みを変える**場面がある。ahc063 では偶数位置
  (bite 直後の eat が critical) の lookahead 重みを倍に。
- **高価な評価はキャッシュ**。ahc063 は 2nd-5th food chain を
  `(cp, 1st_food_pos, food_hash)` で direct-mapped cache (4096 entry)。
  hit 率 ~50% で eval コストほぼ半減。

### 2.6 容量設計

- `tour_capacity` は `beam_width * 深さ` のオーダ。`reserve` のみなので
  超えると vector の倍増戦略でリアロケートが走る (O(N) コピーが数回)。
  余裕を持って確保 (ahc063 は `bw * 100`)。
- `hash_map_capacity` (Hash 版のみ) は open addressing なので実利用数の
  2〜4 倍を推奨。`beam_width * 32 + 1` が一つの目安。

---

## 3. 実装パターン

### 3.1 Arena Pool (可変長 undo)

1 transition あたりの差分サイズが動的な state (1 手で複数駒反転、
連鎖消去、単位伝播、グラフのバルク辺変更など) では、各 UndoInfo に
`vector<Entry>` を持たせると heap 割当が遅い。代わりに共有アリーナ +
`(start, count)` 参照が定石:

```cpp
template <typename Entry, int CAP>
struct ArenaPool {
    Entry buf[CAP];
    int top = 0;
    int mark() const { return top; }
    void push(const Entry& e) { buf[top++] = e; }
    void rewind(int m) { top = m; }
};

struct UndoInfo {
    int arena_start;  // pool 上の開始位置
    int count;        // このフレームで積んだ件数
    // 固定サイズの差分はそのままここに
};
```

- `move_forward` 開始時に `arena_start = pool.mark()` し、差分ごとに
  `push()`
- `move_backward` で `pool.rewind(arena_start)`
- `commit` (undo しないと分かっている場面) では `pool.rewind(arena_start)`
  しても OK。**undo 情報を捨てるだけで pool を前倒し解放できる**

**注意**: pool の容量は `max_turn * 1 手の最大差分数` を想定。ahc063 は
`BITE_POOL_CAP = max_turn × max_snake_len (≈ 100000×200 = 20M)` が必要
だった。初期値を小さく設定すると深い探索木で `OVERFLOW` → SIGABRT
になるので、overflow 検知ログを cerr に出しておくと原因特定が速い。

### 3.2 commit API (undo が重い state 向け)

`EdgeBeamSearch` の Tree は **全候補が共通 prefix を持つとき**、その
prefix を `direct_road_` として確定して post-order 配列から外す最適化を
持つ (direct road advance)。標準実装は `state_.move_forward(action)` を
呼ぶが、以下のようなケースでは undo 情報がゴミとして残り続け、pool を
圧迫する:

- 1 手あたり多数のエントリを積む state (ahc063 の噛みちぎり)
- undo に実質コストがかかる state

対策として State に `commit(action)` を追加し、edge_beam 側の direct road
advance でそちらを呼ぶように差し替える (`lib/edge_beam.cpp` 内で
`state_.commit(action)`):

```cpp
void commit(Action dir) {
    move_forward(dir);
    auto& undo = stack[top - 1];
    pool.rewind(undo.arena_start); // 戻さないので即解放
    undo.count = 0;                 // 誤 backward 時の安全策
}
```

本テンプレの `lib/edge_beam.cpp` には **commit API は入っていない**
(direct road は `move_forward` を呼ぶ)。必要になった場合のみ State 側に
`commit` を足し、edge_beam の 2 箇所の direct road advance を差し替える。

---

## 4. ahc063 からの learnings (一般化できるもの)

- **主項 10000× 倍ルール**: 優先順位を絶対化したい指標は 4 桁係数で
  他項を圧倒する。タイブレークだけが他項に委ねられる。
- **段階 lookahead + cache**: 2-5 段の greedy chain で評価に前方情報を
  入れる。ただし段数を増やすほど hash 多様性を食うので 5 段程度で頭打ち。
  chain score は `(cp, 1st食料 pos, food_hash)` 程度でキャッシュすると
  hit 率 ~50%。
- **偶数/奇数位置の重み差**: bite 直後の eat 距離が critical、などの
  構造を評価関数に折り込む。
- **bw 初期値は問題規模で分岐**: 小規模は小 bw + 拡大、大規模は安全側の
  大 bw で一発。Optuna で N ごとにチューニング。
- **bw 更新ルール**: 失敗時 ×1.1〜1.2、成功時は据え置いてノイズ再出発。
- **hash は粗く**: 状態表現を直接 hash すると dedup 効かない。
  「意味的に異なる状態要素」だけ拾う。
- **max_turn は strict improvement only**: タイ解をカットし、スコア
  改善する場合のみ探索継続。
- **stagnation cutoff**: 進捗が M 手改善しなければ打ち切り。
- **BITE_POOL 型 overflow は実測検知**: 理論最大で確保できない場合、
  cerr に overflow ログを出して seed 特定できるようにしておく。
- **local と lambda の探索量を揃える**: 速度比を実測して TimeKeeper で
  補正し、local 結果と lambda 結果を直接比較可能に。
