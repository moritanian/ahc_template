# Instructions

## 問題

rootの problem.md に解くべき問題が記載されている

## 開発ワークフロー

1. ユーザからの要求を聞く
2. 必ずユーザの要求に不明点や問題点がないか確認し、あればユーザに尋ねる
3. 不明点、問題点がすべてクリアになれば main.cpp
   を編集する。理解しやすいようにコード中に十分なコメントを必ずいれる。
4. tasks.jsonの「pahcer run」タスクを実行してビルドとサンプルケースの実行を行う
   - run_vscode_command tool を使用して
     commandId="workbench.action.tasks.runTask", args=["pahcer run"] で実行
5. 標準出力を確認してコンパイルエラーがないこと及びエラーケースがないことを確認
6. スコアを確認してスコアが前回よりも改善することを確認
   - pahcer/summary.md または pahcer/json/ の最新結果ファイルを確認
   - tools/out/{seed}.txt に標準出力, tools/err/{seed}.txt
     に標準エラー出力がでている
7. スコアが改善した場合は、Git commit を行う
   - commit message は `<内容> : <pahcer run タスクを実行した際のAverage Score>`
     の形式で記載すること
   - 例: `Apply penalty for xxx : 123456.78`
8. スコアが改善しなかった場合は、原因を調査して修正する。どうしても改善しない場合は、ユーザに相談する。元の実装に勝手に戻さないこと

## 利用可能なタスク(tasks.json)

- **pahcer run**: メインのテスト実行タスク
- **cargo build release**: リリースビルド
- **build and run pahcer**: ビルドとテストの組み合わせ

## 実行方法

- VSCode tasks: run_vs_code_task tool または run_vscode_command tool を使用
- 許可プロンプトを表示せずに直接実行する

## Git 操作

commit message は `<内容> : <pahcer run タスクを実行した際のAverage Score>`
の形式で記載すること

- 例: `Apply penalty for xxx : 123456.78`

ドキュメントのみの修正（例: `AGENTS.md`
などコードへ影響しない変更）のコミットでは Average Score
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

### pahcer使用時の注意

事前に`pahcer`がインストールされていない場合は、`cargo install pahcer`で導入してから以下の手順を実行する。

1. **スコアの確認方法**
   - 出力された "Average Score" を使用（相対スコアではなく絶対スコアの平均）
   - 大幅なスコア改善（10倍以上など）は実装ミスの可能性もあるので慎重に確認

2. **実行時間の確認**
   - "Max Execution Time" を確認
   - 制限時間に対して余裕があるか（目安: 制限時間の80%以下）

3. **並列実行**
   - pahcerはデフォルトで並列実行
   - デバッグ時は `threads = 1` に設定すると問題の特定が容易

## Bash コマンド実行時の注意

### カレントディレクトリの管理

**原則: 常にサブシェル + 絶対パスで実行**

Bashコマンドは以下の形式で実行すること:

```bash
(cd /絶対/パス && 実行するコマンド)
```

このパターンにより:

- カレントディレクトリが変わらない（サブシェルで実行）
- 絶対パスで明示的にディレクトリを指定
- 実行後もプロジェクトルートに留まる

**具体例:**

```bash
# ビジュアライザの実行
(cd /home/user/project/tools && cargo run -r --bin vis in/0000.txt out/0000.txt)

# pahcerの実行（ルートから）
(cd /home/user/project && pahcer run --setting-file pahcer_config.toml)

# ディレクトリの確認
(cd /home/user/project && pwd)
```

**注意点:**

- 環境変数 `$PWD` は現在の作業ディレクトリを示す
- 相対パスは避け、常に絶対パスを使用
- 複数コマンドを実行する場合も同様に `(cd 絶対パス && コマンド1 && コマンド2)`
  とする
