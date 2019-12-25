---
layout: page
title: SP17MP1
permalink: /MP1
---

* TOC
{:toc}

# Machine Problem 1 - 使用 MD5 進行版本控制

- Due Date: 11:59 PM, 2017/10/10, 遲交一天扣 5 % ，遲交七天以上零分，亦即 11:59 PM, 2017/10/17 後零分。

---

## 程式名稱
我們將在本作業使用目前上課學到的系統呼叫（read、write、lseek 等等）來實作一個 git 的超級弱化版。這支版本控制的程式該取什麼命字才好呢？[維基百科](https://zh.wikipedia.org/wiki/Git#.E5.91.BD.E5.90.8D.E6.9D.A5.E6.BA.90)告訴我們原來 git 是 Linus 的自嘲，其意義是「混帳」。但助教連當混帳都不配，就是個人生的輸家，所以我們這次編譯出來的程式請命名爲 "loser" 。

## 功能

loser 支援三個子指令（都類似於 git ）： status(顯示目前的修改), commit(提交目前的修改), log(顯示過去提交的歷史) 。

不同於 git 能夠記錄過往的所有歷史而擁有自由自在回到任一 commit 的能力， loser 只能夠知道某些檔案曾被修改過而已，並且不需要處理檔案被移除的問題。

loser 的運作原理十分簡單，它會將過去所有訊息記錄於所求目錄下的 .loser\_record 檔案，所有的子指令都離不開這個檔案，每次 commit 會新增一組記錄，而 status 會去讀取最後一條 commit 記錄以與當前狀態進行比對，log 則根據要求輸出 ./loser\_record 的資料。

但是因爲我們不用像 git 一樣需要回到過去的 commit ，記錄過往的所有資料太浪費空間，我們只需要比較過去檔案的 [MD5](https://zh.wikipedia.org/wiki/MD5) 摘要即可知道是否產生過變化。

此外，類似於 git 可以用 .gitconfig 這個設定檔來做許多初始設定， loser 也會去讀取 .loser_config 來做設定。

### .loser_record

.loser\_record 有一項特質，作爲一種元數據，它**不會記錄自身的修改**，也就是說雖然執行 `./loser commit` 就可能會改變 .loser\_record ，仍舊無需記錄它發生了什麼改變，也無需計算它的 MD5。

在定義 .loser_record 格式之前，我們先定義每個 commit 的格式（[new_file]、[modified]、[copied]的意義於 loser status 一節中說明）：
```
# commit <第幾次>
[new_file]
<新檔案檔名1>
<新檔案檔名2>
.
.
[modified]
<被修改檔案檔名1>
<被修改檔案檔名2>
.
.
[copied]
<原檔名1> => <新檔名1>
<原檔名2> => <新檔名2>
.
.
(MD5)
<檔名1> <檔名1代表之檔案所產生的MD5>
<檔名2> <檔名2代表之檔案所產生的MD5>
.
.
```
- [new_file]、[modified]、[copied]、(MD5) 以下都會跟隨一行行字串，每一行都該**遵循字典順序由小到大排序**（可使用 strcmp）。
- [copied] 中的箭號(=>)兩側有空白

而 .loser_record 不過就是各個 commit 以**空行做間隔**串接起來而已：
```
<# commit 1 內容>

<# commit 2 內容>

<# commit 3 內容>

.
.
.

<# commit N 內容>
```

### 子指令

loser 支援三個子指令

```
loser status <目錄>
loser commit <目錄>
loser log <數量> <目錄>
```

### loser status

#### 描述
```
loser status <目錄>
```
`loser status` 是最簡單也最常用的子指令，它能夠追蹤並顯示所求目錄相較於**上一次 commit （不需考慮更久之前的 commit）** 產生了哪些變化，我們將這些變化分類爲：

1. new_file： 檔名在上一次 commit 沒有出現過，並且不符合[copied]的條件。
2. modified： 檔名在上一次 commit 中出現過，但 MD5 的結果與上次不同。
3. copied： 檔名在上一次 commit 沒有出現，但 MD5 的結果與上次 commit 中的某個檔案相同。

**保證不會有刪除檔案的狀況發生**，因此無需處理「刪除」的狀況。

具體的輸出格式其實就是一個 commit 記錄去掉編號與 MD5 部分：

```
[new_file]
<新檔案檔名1>
<新檔案檔名2>
.
.
[modified]
<被修改檔案檔名1>
<被修改檔案檔名2>
.
.
[copied]
<原檔名1> => <新檔名1>
<原檔名2> => <新檔名2>
.
.
```

我們特別來看一下[copied]：
```
[copied]
<原檔名1> => <新檔名1>
<原檔名2> => <新檔名2>
.
.
```
很可能會發生一個檔案的 MD5 與上一次 commit 中不只一個檔案的 MD5 相同的情形，那此時的<原檔名>只要任意添入其中一個就好。


同樣記得[new_file]、[modified]、[copied] 以下都會跟隨一行行字串，每一行都該**遵循字典順序由小到大排序**（可使用 strcmp）。

#### 特殊情形

- .loser_record 檔案不存在：視所有檔案爲新檔案
- 檔案與上一次 commit 沒有任何不同：輸出
	```
	[new_file]
	[modified]
	[copied]
	```


### loser commit

#### 描述
```
loser commit <目錄>
```
每次執行會計算與上次 commit 的差異並追加到 .loser_record 檔案的末尾。

格式見 [.loser_record](#.loser_record) 一節。

[new\_file]、[modified]、[copied]的規則與 loser status 一節所述相同， MD5 部分則記錄**目錄底下的所有檔案**（.loser_record 除外）與其 MD5 的對應。

同樣這些 MD5 記錄都該**遵循字典順序由小到大排序**（可使用 strcmp）。

#### 特殊情形

- 檔案與上一次 commit 沒有任何不同：不記錄 commit ，也就是不做任何操作（**注意此與 `loser status` 行爲不同**）
- .loser_record 檔案不存在、但存在其他檔案：建立 .loser_record 檔案（權限爲該檔案擁有者可讀可寫），並視所有檔案爲新檔案
- .loser_record 檔案不存在、也不存在任何其他檔案：依然不做任何動作，連 .loser_record 檔案都不建立

### loser log

#### 描述
```
loser log <數量> <目錄>
```
log 子指令會接一個 \<數量\> ，表示該輸出 .loser\_record 檔案中倒數 \<數量\> 個 commit 資訊。 這些資訊與 .loser\_record 檔案記錄的資訊完全相同，但順序相反，換句話說，越新的 commit 越先輸出。

\<數量\>保證爲一個數字，

舉個例子，假如 .loser_recod 的內容爲：

```
# commit 1
[new_file]
a
[modified]
[copied]
(MD5)
a 900150983cd24fb0d6963f7d28e17f72

# commit 2
[new_file]
[modified]
a
[copied]
(MD5)
a e2fc714c4727ee9395f324cd2e7f331f

# commit 3
[new_file]
b
[modified]
[copied]
(MD5)
a e2fc714c4727ee9395f324cd2e7f331f
b d9cff842fcbed17968931bc9eb97a826
```
則

`./loser log 2 <該目錄>` 該輸出

```
# commit 3
[new_file]
b
[modified]
[copied]
(MD5)
a e2fc714c4727ee9395f324cd2e7f331f
b d9cff842fcbed17968931bc9eb97a826

# commit 2
[new_file]
[modified]
a
[copied]
(MD5)
a e2fc714c4727ee9395f324cd2e7f331f
```

#### 特殊情形

- .loser\_record 檔案不存在~~或裡面沒有任何 commit~~（9/27補充：.loser_record 只要存在就至少包含一個 commit） ：不輸出任何東西
- `loser log <數量> <目錄>` 的數量大於目前 commit 的數量：輸出所有歷史

### .loser_config

**注意！ .loser_config 檔案不同於 .loser_record ，status 應顯示此檔案資訊， commit 時也該記錄此檔案資訊**

./loser 執行時必須去查找作用的目錄底下是否存在 .loser_config 檔案，若不存在則不予理會，若存在則解析之。

.loser_config 只需支援別名功能，也就是說我們若設定 .loser_config 爲：

```
st = status
ci = commit
```

那執行 `./loser st` 等同於執行 `./loser status` ，執行 `./loser ci` 等同於執行 `./loser commit`，以此類推。

.loser_config 詳細規格爲：
```
<別名1> = <子指令名1>
<別名2> = <子指令名2>
.
.
```

- 別名宣告之間不允許空行
- 別名長度不超過 256 個位元組
- （9/27 補充）別名不允許爲子指令名稱 "status", "commit", "log"
- 不能有一個別名代表多個子指令（例如 `QQ = status` 又 `QQ = log`）
- 一個子指令可能會有多個別名
- 每個別名宣告中的等號(=)兩側有空白

## 更多範例

參考[這裡](EXAMPLE.md)

## 實作細節

### 輸入輸出

`./loser status`、`./loser log` 都寫至標準輸出，`./loser commit` 寫至 ~~.loser_onfig~~ .loser_record。

測試時也會檢查標準輸出與 .loser_record，以及是否有其他檔案受到修改，但標準錯誤輸出可以自己拿去印東西來協助除錯。

### 讀取目錄

由於目前課程尚未講授讀取目錄的系統呼叫，故助教提供完成此一功能的[程式碼](/mp1/src/list_file.zip)，使用方法寫在註解中，同學們不一定要使用助教提供的程式碼，可以自行撰寫。

### MD5

演算法請參考[維基百科](https://zh.wikipedia.org/wiki/MD5)。

小提醒是如果不確定 `unsigned int` 到底是多少位元，可以 `#include <stdint.h>` 來取用 `uint32_t` 跟 `uint64_t` 來明確使用 32 位元跟 64 位元的資料型別。

又爲降低同學的負擔， MD5 可以使用上網搜尋程式碼來參考(google "md5 c")，但請自行驗證其正確性並承擔風險。

### 效能問題

注意到 `loser status`、`loser commit` 都會去讀取最後一筆 commit，而 `loser log` 輸出的順序也與 .loser_conig 儲存檔案的順序顛倒，由於有大量操作都發生在檔案尾端，採用 `lseek` 函式能夠顯著提高效能。

## 評分

共 8 分
1. (2 分) `loser status` 輸出正確狀態
2. (2 分) `loser commit` 正確寫入 .loser_record
3. (2 分) `loser log` 輸出正確歷史資訊
4. (2 分) 測資綜合測試三個子指令，並且程式能夠讀取 .loser_config 使用給定別名

注意到 `loser status`、`loser commit`、`loser log`三個指令可以完全讀立運作，因此測試一個子指令時不會測試到其他子指令。同學也可以僅實作部分功能以獲得部分分數。

第四項綜合測試時才會將三個指令混合起來測試，因此只要三個子指令其一有誤就拿不下第四項的分數。

### 測試資料的保證

- 不刪除檔案
- .loser_record 若存在必符合格式並可寫
- .loser_record 檔案小於 2GB
- .loser_config 若存在必符合格式
- .loser_config 中不超過 1000 個別名
- .loser_config 的別名由小寫英文字母組成
- 各子指令參數的<目錄>必存在且可讀可入
- 作用目錄不包含任何目錄，只包含檔案
- 所有檔案的名稱皆由英文字母大小寫、數字、底線(_)、點(.)組成，並且長度小於 255 位元組
- 作用目錄底下不超過 1000 個檔案
- 作用目錄底下所有檔案皆可讀
- 作用目錄底下檔案大小總和不超過 100MB
- `loser log` 輸出的資料量小於 10MB

### 公開測試資料

請見 [公開測試資料](judge.md)

### 時間限制

任何一個子指令操作時間必須小於 5 秒。


## 繳交方式

與 MP0 雷同，請在同一個 repo 下建立另一個 MP1 ，將 Makefile 與程式碼置於其中。

```
repo
├── MP0
│   ├── Makefile
│   └── other files
└── MP1
    ├── Makefile
    └── other files
```
我們會在 MP1 目錄下執行 `make` ，並測試此指令生成的 loser 執行檔。


## 有用連結

- [MD5 - 維基百科，自由的百科全書](https://zh.wikipedia.org/wiki/MD5)
- [讀取目錄輔助程式碼](/mp1/src/list_file.zip)
- [範例頁](EXAMPLE.md)
