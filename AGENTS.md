# Instructions

## 問題

rootの problem.md に解くべき問題が記載されている

## 開発ワークフロー

1. ユーザからの要求を聞く
2. 必ずユーザの要求に不明点や問題点がないか確認し、あればユーザに尋ねる
3. 不明点、問題点がすべてクリアになれば main.cpp を編集する
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

commit message は `<内容> : <pahcer run タスクを実行した際のAveage Score>`
の形式で記載すること

- 例: `Apply penalty for xxx : 123456.78`
