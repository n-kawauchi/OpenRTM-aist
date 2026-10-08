#!/bin/bash

# 1. 置換対象となるファイルの確認（バックアップ前のチェック）
echo "--- 置換対象ファイル ---" 
find . -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.h" \) -exec grep -l "FastRTPS" {} +

# 2. .bak 拡張子でバックアップを作成しつつ一括置換
find . -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.h" \) -exec sed -i.bak 's/FastRTPS/FastDDS/g' {} +

echo "--- 置換処理が完了しました（元ファイルは .bak として保存されています） ---" 
