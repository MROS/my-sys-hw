# MP1 範例

## 第一次 commit
假設現在我們把 loser 寫好了，目錄結構長這樣：
```
.
├── loser
└── test
    └── a.txt
```
而 a.txt 裡面就只有**一個字元** a （沒有空行）：
```
a
```
那此時執行
```
./loser status test
```
因爲 test 目錄中沒有 .loser_record 所以會將所有檔案當作新檔案：
```
[new_file]
a.txt
[modified]
[copied]
```
執行
```
./loser log 3 test
```
雖然要求看近期三個 commit 記錄，但因爲根本沒有 .loser_record 所以完全不輸出東西

執行
```
./loser commit test
```
不輸出任何東西，但此時的 test 目錄會新增一個 .loser_record 檔案其內容爲：
```
# commit 1
[new_file]
a.txt
[modified]
[copied]
(MD5)
a.txt 0cc175b9c0f1b6a831c399e269772661
```

## 第二次 commit

接續之前，我們對 test 目錄底下的檔案再做一些操作
``` sh
cp test/a.txt test/_a.txt
echo "st = status" > test/.loser_config
```
現在的目錄結構應該是：
```
.
├── loser
└── test
    ├── _a.txt
    ├── a.txt
    ├── .loser_config
    └── .loser_record
```
由於設定了 .loser_config ，可以用 `loser st` 來代替 `loser status` 了
```
./loser st test
```
得到：
```
[new_file]
.loser_config
[modified]
[copied]
a.txt => _a.txt
```
執行
```
./loser log 3 test
```
雖然要求看三個 commit 記錄，但目前只有一則 commit 記錄，所以盡數輸出：
```
# commit 1
[new_file]
a.txt
[modified]
[copied]
(MD5)
a.txt 0cc175b9c0f1b6a831c399e269772661
```
執行
```
./loser commit test
```
此時 .loser_record 會變成
```
# commit 1
[new_file]
a.txt
[modified]
[copied]
(MD5)
a.txt 0cc175b9c0f1b6a831c399e269772661

# commit 2
[new_file]
.loser_config
[modified]
[copied]
a.txt => _a.txt
(MD5)
.loser_config 759656817c316c8cb8f1d276ae324f0b
_a.txt 0cc175b9c0f1b6a831c399e269772661
a.txt 0cc175b9c0f1b6a831c399e269772661
```
注意到 .loser_config、_a.txt、a.txt 是照字典序排序

## 下載範例目錄

點此[下載](example/test.zip)，若有空白或空行等格式不甚確定可以觀察本目錄