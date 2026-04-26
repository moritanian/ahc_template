# Instructions

## 問題

rootの problem.md に解くべき問題が記載されている

## 開発ワークフロー

1. ユーザからの要求を聞く
2. 必ずユーザの要求に不明点や問題点がないか確認し、あればユーザに尋ねる
3. 不明点、問題点がすべてクリアになれば main.cpp
   を編集する。理解しやすいようにコード中に十分なコメントを必ずいれる。
4. 動作確認として `phst run` を実行（デフォルト seed 0 から 100 ケース）。
   コンパイルエラー・WA がないこと、暴走していないことを確認する。
   - `phst run` — seed 0 から 100 ケース実行
   - `phst run -s 42` — seed 42 のみ実行
   - `phst run -s 10:20` — seed 10〜20 を実行（inclusive）
   - `phst run -s 10 -n 50` — seed 10 から 50 ケース（seed 10〜59）実行
5. 標準出力を確認してコンパイルエラーがないこと及びエラーケースがないことを確認
   - tools/out/{seed}.txt に標準出力, tools/err/{seed}.txt
     に標準エラー出力がでている
6. **評価用 N ケース実行**: 動作確認が通ったら、以下のコマンドで local
   で N ケースを実行する。
   ```
   phst run -n <N> -c "<適切なコメント>"
   ```
   - **N の選び方 (デフォルト: 100)**:
     - 短期コンテスト / 長期コンテストの序盤〜中盤 → **N=100** (回転率優先)
     - 長期コンテストの終盤、or 改善幅が小さくノイズに埋もれそうな変更
       → **N=1000** (統計的有意性重視)
     - 迷ったら 100 から始め、差が微妙なら 1000 で再評価
   - `<適切なコメント>` には今回の変更内容を短く説明する文字列を入れる
     （例: `noise mask=7, growth 1.2`）。`phst results list` の Comment
     列で後から区別できるようにするため必須。
   - **local 実行がデフォルト**。`pahcer_config.toml` で `-DLOCAL` が付くため、
     `TimeKeeper(<local_ms>)` が適用され、AtCoder 時間相当の探索量に制限される。
   - **lambda はデフォルト不使用**。大量ケース並列実行が必要で local だと
     時間がかかりすぎる等の事情があるときのみ、ユーザに相談のうえで使用する。
     使う場合は `phst run -n <N> --lambda -c "..."`。`-DLAMBDA` により
     `TimeKeeper(<lambda_ms>)` が適用され、local と同等の探索量になるよう
     調整済みで、lambda 結果は local 結果と直接比較可能。
7. スコア判定: **過去の実行結果と relative score を比較して改善したか確認する**。
   - `phst results list --limit 20` で過去の run を確認する。
   - **比較は同じ N 同士で行う**のが原則 (N=100 と N=1000 は seed 集合が
     異なるため直接比較しない)。Cases 列で絞り込む。
   - 比較対象は直近 1 件だけでなく、近い文脈の過去実行（同じブランチ・
     同じ戦略方針のもの）も確認する。
   - **relative score の注意 (重要)**:
     - pahcer の relative score は **全 run の best scores を基に常に再計算** される
     - 新しい run を実行すると、**過去の run の relative score も変わる**
     - **比較は必ず `phst results list` の最新出力で行うこと**
     - lambda 実行直後のログに表示される relative score は**その時点の値**であり、
       後の run で変動するため、**ログの値をメモして比較してはいけない**
     - 改善/悪化の判断は毎回 `phst results list --limit N` を実行して
       最新の relative score 同士を比較すること
8. relative score が改善した場合は Git commit を行う。
   - commit message は
     `<内容> (N=<N>) : <local 実行の relative score (%)>`
     の形式で、**N を明記する**。
   - 例: `Apply penalty for xxx (N=100) : 92.04%`
   - 例: `Tune beam width (N=1000) : 85.83%`
9. relative score が改善しなかった場合は、原因を調査して修正する。どうしても改善しない場合は、ユーザに相談する。元の実装に勝手に戻さないこと


## 実行環境と時間設定

### TimeKeeper 設定 (`-DLOCAL` / `-DLAMBDA` で自動切替)

`main.cpp` で `#ifdef LOCAL` / `#elif defined(LAMBDA)` / `#else` により
TimeKeeper 値を切り替える。local と lambda は実測で同等の探索量になる
ように調整する（lambda は local より遅いので TK を伸ばす）。

| 環境 | flag | 設定方法 |
|---|---|---|
| local (phst) | `-DLOCAL` | `pahcer_config.toml` で自動付与 |
| lambda (phst --lambda) | `-DLAMBDA` | `pahcer_config.toml` で自動付与 |
| AtCoder 提出 | なし | デフォルト |

- 具体的な ms 値はコンテストごとに計測して決める。
- 手動変更は不要。提出時はそのまま提出すれば AtCoder 設定になる。


## Git 操作

commit message は
`<内容> (N=<N>) : <local N ケース実行の relative score (%)>`
の形式で、**N (評価ケース数) を明記する**こと。

- 例: `Apply penalty for xxx (N=100) : 92.04%`
- 例: `Tune beam width (N=1000) : 85.83%`

relative score は `phst run -n <N> -c "..."` (local 実行、lambda は
デフォルト不使用) の結果の `Average Relative Score` をそのまま使う。

ドキュメントのみの修正（例: `AGENTS.md`
などコードへ影響しない変更）のコミットでは relative score
の記載は不要で、通常のメッセージで構わない。

## デバッグ・トラブルシューティング

### WAが出た場合

1. **出力ファイルを確認**
   - `tools/out/{seed}.txt`: 標準出力（プログラムの出力）
   - `tools/err/{seed}.txt`: 標準エラー出力（デバッグログ）

2. **ビジュアライザで詳細確認**
   ```bash
   (cd /absolute/path/to/project/tools && cargo run -r --bin vis in/{seed}.txt out/{seed}.txt)
   ```
   - エラーメッセージから問題箇所を特定
   - "Out of range" などは出力形式や制約違反を示す
   - 絶対パスは自分のプロジェクトパスに置き換える

3. **問題文の制約を再確認**
   - 出力値の範囲（0-indexed, 上限値など）
   - 出力形式（スペース区切り、改行位置など）
   - 特殊ケースの扱い（最初/最後の要素、境界値など）

### 戦略実装時の確認ポイント

1. **実装前に必ず確認**
   - 戦略文書の曖昧な表現や不明点をユーザに質問
   - 用語の定義（経路長、遷移回数、インデックスなど）
   - 境界条件の扱い（最初/最後の要素、ループの終了条件など）

2. **実装後のチェック**
   - 問題文の全制約を満たしているか
   - エッジケース（最小/最大値、空配列など）の動作確認
   - ループ変数のオーバーフロー・アンダーフローチェック


## Bash コマンド実行時の注意
- user に毎回許可を求めることを回避するため、 `()` をつけない:
  - 悪い例: `(phst run)`
  - 良い例: `phst run`

## グラフ・解析ファイルの保存ルール
- グラフ (PNG, PDF など) と解析結果 (CSV, JSON など) は **`analysis/`
  ディレクトリ以下に保存する**こと。リポジトリルート直下に大量のファイルを
  生成しないこと。
  - 悪い例: `df.to_csv("stats.csv")`
  - 良い例: `df.to_csv("analysis/stats.csv")`
- グラフを生成・表示した際は、**必ずファイル名を明記する**こと。
  - 例: 「グラフを `analysis/bw_compare_N.png` に保存しました」
  - グラフ生成スクリプト内でも `print(f"saved {filename}")` 等で出力する。
- スクリプトは `scripts/` 以下に置き、デフォルト出力先を `analysis/` にする
  こと。

## 一時/中間ファイルの取り扱い
- デバッグ用バイナリ (`main_debug*`)、`a_*.out`、`*.prof`、`err_debug.txt` 等
  はリポジトリにコミットしない。`.gitignore` に追加するか、作業終了時に削除。
