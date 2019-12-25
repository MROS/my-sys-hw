# MP1 公開測試資料

## 下載

點此[下載](judge/mp1_judge.zip)，若有空白或空行等格式不甚確定可以觀察本目錄

## 設置

先確定裝有 v8.0 以上的 node.js。具體安裝方式請見[官網](https://nodejs.org/en/download/package-manager/)

(工作站已經裝好了，建議前往工作站)

``` sh
# 假設現在處於一個空目錄底下
# 下載該檔案
wget https://systemprogrammingatntu.github.io/mp1/judge/mp1_judge.zip
# 解壓縮
unzip mp1_judge.zip
# 刪除 zip 檔案（可選）
rm mp1_judge.zip
# 安裝該程式所依賴的所有套件庫
npm install
```

## 創建測資

```
./judge.js create
```

純粹創建測資與答案。創建兩個目錄，一個是 data/ ，另一個是 standard_answer/

data/ 跟 standard\_answer/ 底下都含有七個對應的目錄，standard\_answer 底下的各個目錄底下都各有三個檔案：status, log, commit。（1\_empty 是例外，它沒有 commit 檔案 ，因爲 loser commit 該目錄後不該生成 .loser\_record）

這三個檔案分別對應 ./loser status 的標準輸出內容、./loser log 的標準輸出內容、./loser commit 之後所生成的 .loser_recor。

各位可以以這些測試資料自行進行測試。

```
./judge.js clean
```
清除本腳本生成的檔案


## 執行測試腳本

在測試你的程式之前，請先將生成的 loser 執行檔丟入適才執行解壓縮的那個目錄中

``` sh
cp 某/處/loser .
```

請記得在執行以下任一指令前都先跑過 `./judge.js clean`，因爲 commit 會造成破壞性效果，因此不清除會有問題。

但不需要額外跑一次  `./judge.js create` ，這些指令都會自動創建測資。
```
./judge.js            # 測試所有並跑分
./judge.js all        # 測試所有並跑分，與 ./judge.js 同效果
./judge.js log        # 僅測試 log
./judge.js status     # 僅測試 status
./judge.js commit     # 僅測試 commit
./judge.js alias      # 僅測試有別名的測資
```

## 測資說明

七項測資中，4, 5, 6 是較大型的測資，較有超時的可能。
測資共七項，分別爲

- 1_empty
- 2_only_a
- 3_commit_2
- 4_one_large_file
- 5_many_files
- 6_large_record
- 7_alias

1 ~ 3 爲小測資，4, 5, 6 可能稍微耗時，7 則專測別名，也是很小的測資。

因爲大測資的檔案不小，下載會極爲耗時而且可能打開後不明所以，所以撰寫了腳本，通過觀看腳本也許能更有效的理解檔案的內容，在本地生成也能加快速度，但仍然不夠快，爲免同學久等，我事先調低了第六個測資的檔案大小，若想測試最大的檔案，請在 `config.js` 中修改設定。

若執行 `./judge.js` 或 `./judge.js all` ，前六個測資全對才會執行第七個測資，並取得別名的分數。

但同學們依然可以執行 `./judge.js alias` 來直接測試別名功能。

正式評分時仍有可能加入其他測資，但不會再有考驗效能的測資了。