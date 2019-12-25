---
layout: page
title: SP17MP4
permalink: /MP4
---

* TOC
{:toc}

# Machine Problem 4 - 多進程／線程伺服器與客戶端

- Due Date: 11:59 PM, 2017/12/26, 遲交一天扣 5 % ，遲交七天以上零分，亦即 11:59 PM, 2018/01/02 後零分。
- 作業問題請至 [MP4 issue](https://github.com/SystemProgrammingatNTU/mp4-discussion/issues) 討論

---

## 引言

助教唸大學的時候，匿名聊天有一陣子滿風行的，最先是有人在 PTT 分享了他在 [liveany](https://www.liveany.com/) 上的一次愉快經驗，鄉民們對此很感興趣，這篇文章成了爆文之後，突然之間有一堆人開始玩匿名聊天，而有需求就有供應，匿名聊天的網站像是雨後春筍一般冒了出來，展開了一場匿名聊天網站大戰，其中 [Wootalk](https://wootalk.today/) 可說是這場大戰的勝利者，完全取代了 liveany 的主流位置。

而助教當年所主導開發的 bonbon 則有若流星劃過天際，稍縱即逝......

這一次，同學們的任務就是延續當年那次互聯網大戰，開發出自己的匿名聊天系統。

## 簡介

市面上的匿名聊天是非常簡單的系統，它的功能就只是，讓不同的客戶端連線到伺服器，並以先來後到的順序爲客戶端進行匹配，而完成匹配之後所發送的訊息就會送到匹配對象處。

而本次作業需要實作一項市面上匿名聊天系統所沒有的功能：可以透過撰寫程式來決定匹配對象。

當客戶端連上伺服器之後，必須提供自己的基本資料：昵稱、年紀、性別、自我介紹、篩選函式以要求伺服器開始幫它配對，之後伺服器會一一檢驗位於等待佇列中的候選者與該客戶端是否互相符合對方的篩選函式，若符合，將候選者移出佇列，並告訴兩者配對成功。

篩選函式的點子來自於一個新興論壇——[無限城](https://city-of-infinity.com)（這個論壇不管是文章還是留言都可以寫成函式，而且也可以用函式來過濾留言），因此本次作業所生成的執行檔名稱就是無限城與 bonbon 的綜合體： `inf-bonbon-server` 。

## 執行流程

由於伺服器是一個網路程式，必須綁定埠口，因此得接上一個參數。ip 位址則無需顯式給出，伺服器應自行綁定於 0.0.0.0 （也就是說只要連到本臺電腦就接受，更多網路通訊的資料請看實作技巧中的 [socket教學](#socket教學)）

``` sh
./inf-bonbon-server [port]
```

助教提供了一支客戶端程式方便同學進行測試，使用前先確定裝有 v8.0 以上的 node.js。具體安裝方式請見[官網](https://nodejs.org/en/download/package-manager/)

(系上工作站已經裝好了，建議前往工作站)

``` sh
# 假設現在在一個空目錄中
wget https://systemprogrammingatntu.github.io/mp4/inf-bonbon-client.zip
unzip inf-bonbon-client.zip
npm install
```

編輯個人資料，之後會用於匹配。
```
vim info.json
```
名字、年齡、性別、自我介紹等等應該一目瞭然，篩選函式則請先不做修改，之後會進行說明（見 [API 詳細規格及功能](#API詳細規格及功能)）。

然後我們就可以執行客戶端啦！
```
node client.js [ip] [port]             # 由於 client 可能位於其他機器，因此需指定 ip 位址
```

若成功連上伺服器，會看到客戶端打印出提示
```
已連上伺服器。
請輸入下一步命令
[/t （嘗試匹配）]  [/c （結束網路連線）]
```
輸入 `/t` 後按 enter 就會開始進行配對，若伺服器表明收到請求，則客戶端會顯示。

```
匹配中...
[/q  （放棄匹配）]  [/c （結束網路連線）]
```

當成功匹配到時，客戶端會顯示對方的基本資料以及
```
[/q  （結束聊天）]  [/c  （結束網路連線）]
```
此時除了此兩個指令，你可以打任何字詞跟對方聊天了。

本次同學要實作**伺服器端**（不用繳交客戶端），會有很多客戶端同時連上一個伺服器端，它們之間以 JSON 格式爲 API 進行通訊，而伺服器必須平行化去處理篩選函式，至少得達到兩倍於單 CPU 的效能，同時客戶端可能會上傳惡意的篩選函式，因此伺服器得保證不會因爲惡意篩篩選函式的攻擊而導致服務崩潰。

**注意**，助教提供的客戶端功能有限，當你需要進行更有彈性的測試時，就可能需要撰寫自己的客戶端，此一客戶端可以利用任何語言任何技術來完成。

## API 概述

客戶端與伺服器必須合作無間才能夠完成一個應用，而我們需要透過 API 才能夠使它們合作，我們先在本節概略介紹這些 API 的意義，之後再給出詳細規格。

本作業中的客戶端會發出以下三種指令：
- try_match：告訴伺服器自己的基本資料，並開始配對
- send_message：配對成功後，傳送訊息給對方
- quit：還在配對中但是不想玩了，或是聊天聊到一半不想聊了，請求停止。

伺服器端則可能發出以下六種訊息：
- try\_match：告訴客戶端它收到 try\_match 請求了
- send\_message：告訴客戶端它收到 send\_message 請求了
- quit：告訴客戶端它收到 quit 請求了
- matched：客戶端成功配對時，會傳送 matched ，內含有匹配對象的基本資料
- receive\_message：若 A 與 B 完成配對，若 A 傳送 send\_message 給伺服器，伺服器除了要回傳 send\_message 給 A ，也要送 receive\_message 給 B ，透過此一機制來做到聊天的功能
- other\_side\_quit：若 A 與 B 配對到正在聊天，A 送出 quit 或 A 關閉連線，則應傳送 other\_side\_quit 予 B ，告知對方已經離開。

請注意我們用同樣的名字去稱呼伺服器的前三種 API 與客戶端的 API，伺服器的這三種 API 的用途在於告知客戶端「已收到請求開始處理」，而透過這種機制，客戶端跟伺服器才能保持同步。

## 使用者的狀態轉移

我們用一個狀態機來描述客戶端自身的狀態，圖中的箭頭代表發生事件導致狀態轉移。

![MP4 客戶端用戶狀態機](https://i.imgur.com/gpvKxey.png)

在圖中已經用文字的顏色將可能發生的事件分成兩類
- 藍色：接收到伺服器送來的命令
- 綠色：客戶端主動關閉連線

注意 try\_match, send\_message, quit 都是伺服器送來的指令，表示它已收到客戶端的同名指令，而非客戶端主動送出指令。


而伺服器也會需要維護與記錄每一個使用者的狀態，才能在遇到不同狀況時作出相應的反應。

以下用一個有限狀態機來表達伺服器如何維護**一個使用者**的狀態，圖中的箭頭代表該使用者送出指令給伺服器，或者是伺服器內部發生事件，導致使用者的狀態轉移。

![MP4 伺服器中用戶狀態機](https://i.imgur.com/83foSUI.png)

在圖中已經用文字的顏色將可能發生的事件分成三類

- 藍色：接收到客戶端送出的指令
- 綠色：偵測到客戶端關閉 socket
- 黑色：其他客戶端的事件影響或伺服器完成某事

測試時，本圖中有繪製箭頭的指令才會送出，也就是說，客戶端只會

- 在「閒置中」主動送出 try_match
- 在「配對中」主動送出 quit
- 在「聊天中」主動送出 send_message, quit

回頭看一下助教所提供客戶端的提示，`/t` 代表送出 try\_match ，`/q` 代表送出 quit ， `/c` 代表關閉 socket，而在已匹配到聊天對象時，預設會送出 send\_message。

## API 格式介紹

API 將採用簡單的 JSON 格式，同學需自行解析該格式。

### JSON 簡介

JSON 是一個極爲簡潔又富有表達力的資料交換語言，由於它是 JavaScript 語言的一個子集，並且比 XML 簡單許多，因此在 Web 通信中被廣爲使用。

JSON 極其簡單，較爲權威的說明建議閱讀 [Introducing JSON](https://www.json.org/json-zh.html) 或是 [MDN 中的 JSON 介紹](https://developer.mozilla.org/zh-TW/docs/Web/JavaScript/Reference/Global_Objects/JSON)。

同時，本次作業只會使用到 JSON 的子集，這裡延續 [Introducing JSON](https://www.json.org/json-zh.html) 的術語來說明。

一個標準的 JSON 是一個無序的「名稱／值」對，值可以是字串、數字、物件、陣列、真假值、 null ，而在本作業中，值只能是

- 字串
- 數字
- 真假值

同時，數字只會是正整數。

如此應該是一個能夠輕易解析的結構了，然而考慮到解析字串時仍需要處理跳脫字元，可能有點繁瑣，因此允許同學們使用第三方函式庫。注意到，[Introducing JSON](https://www.json.org/json-zh.html) 底下提供了一個 JSON 函式庫的列表。

其中，[cJSON (C 語言實作)](https://github.com/DaveGamble/cJSON) 與 [JSON for modern C++ (C++ 實作)](https://github.com/nlohmann/json) 可以直接拿它的源碼與自己的程式編譯在一起，適合於繳交作業。

#### 傳輸

 **重要**

在 TCP 協定中，一次 send 不一定會對應到一次 write ，因此客戶端的一次 JSON 請求未必能被一次讀出，需要撰寫邏輯來將不同份的 JSON 分開再進行解析。

而爲了便於解析，我們規定所有的 JSON API 傳遞時都先被轉換成 [line delimited JSON](https://en.wikipedia.org/wiki/JSON_Streaming#Line_delimited_JSON) （行分隔的 JSON），一個 JSON 內部不會含有換行，只會在 JSON 結尾處補上一個~~空白~~換行以方便做切割 。因此同學只要判斷是否遇到換行，即可分割 JSON。

後面小節中介紹 API 時，會爲了閱讀方便不寫成行分隔的 JSON ，但在實際傳輸中，請務必使用行分隔的 JSON。


## API詳細規格及定義

本次的 API 有些龐雜，我們將它分散於三組進行說明，之後我們再統一彙整一次。

這三組分別爲：
- try\_match ：與客戶端主動指令 try\_match 相關連的
- send\_message ：與客戶端主動指令 send\_message 相關連的
- quit ：與客戶端主動指令 quit 相關連的

### try_match 相關

客戶端處於「閒置中」狀態時，可發出 try_match 請求，使伺服器將其排入等待佇列。

``` javascript
{
	"cmd": "try_match",
	"name": "loser",                  // 名字長度不超過 32 字元
	"age": 23,                        // 年齡爲 0 ~ 99（請注意該數字兩側並無雙引號，它的型別是數字而非字串）
	"gender": "male",                 // 性別僅能爲 "female" 或 "male"
	// 自介爲不超過 1024 字元的英文
	"introduction": "I get hurt a lot in that chatroom war...",
	// 篩選函式爲不超過 4096 字元的一個 C 語言函式，詳細說明請繼續往下看
	"filter_function": "int filter_function(struct User user) { return 1; }"
}
```

收到該請求時，需立刻回傳以下訊息，以表示已收到請求
``` javascript
{
	"cmd": "try_match"
}
```

成功匹配時，伺服器傳回對方的資訊
``` javascript
{
	"cmd": "matched",
	"name": "winner",                 // 名字長度不超過 32 字元
	"age": 23,                        // 年齡爲 0 ~ 99（請注意該數字兩側並無雙引號，它的型別是數字而非字串）
	"gender": "female",                 // 性別僅能爲 "female" 或 "male"
	// 自介爲不超過 1024 字元的英文
	"introduction": "so sorry...",
	// 篩選函式爲不超過 4096 字元的一個 C 語言函式，詳細說明請繼續往下看
	"filter_function": "int filter_function(struct User user) { return 1; }"
}
```

#### 篩選函式

篩選函式是一個 C 語言的函式，接收一個型別爲 `struct User` 的參數，回傳值型別爲 `int` 。

struct User 用來代表一個使用者的基本資料，定義如下
``` c
struct User {
	char name[33],
	unsigned int age,
	char gender[7],
	char introduction[1025]
};
```

當 filter_function 回傳 0 時，代表無法與該 User 匹配；回傳其他值時，代表通過篩選。

當兩個使用者互相通過對方的篩選函式，則兩者完成匹配。

請將此結構定義與 filter_function 定義放入一個檔案，以 `gcc -fPIC -O2 -std=c11 [filter_function].c -shared -o [share_lib_name].so` 來生成出動態鏈接函式庫，以供之後載入，**注意，若不以指定參數編譯，可能會導致測試時性能評估錯誤、或是標準不同而編譯失敗**

若~~編譯時發生錯誤（gcc 返回值不爲 0），或是~~執行時進程崩潰，也算成未通過篩選。

敘述一次 filter_function 執行的流程：
1. 自動態函式庫中取出 filter_function
2. 將欲進行匹配的使用者資料取出，製作爲 struct User 型別
3. 把此一 struct User 作爲參數傳遞給 filter_function

給出一個 filter_function 的範例，假如說我們想要篩選到的對象是高中女生，我們可以這樣寫篩選函式：

``` c
int filter_function(struct User user) {
	return user.gender[0] == 'f' && user.age <= 18 && user.age >= 16;
}
```

而將 try_match API 轉換爲 struct User 的規則如下：
1. 去掉 cmd, filter_function 屬性
2. 將 age 轉爲 int
3. 將 name, gender 轉爲 C 的字元陣列，並在末尾補上 '\0'
4. 將 introduction 轉爲 C 的字元陣列，但需注意它可能包含跳脫字元，需轉換。一樣要在末尾補上 '\0'

轉換的過程應該可以透過先前提到的函式庫做到，當然若你手寫了 JSON 解析器，這自然也不是什麼問題了！

關於動態函式庫的更多訊息，請參見[實作技巧](#實作技巧)。

#### 匹配算法

匹配算法採先來先到

1. 若等待隊列中沒有任何人，則該使用者進入等待隊列等候
2. 若等待隊列中有人，則從隊伍頭部開始往後一一嘗試配對
	1. 一配對成功，其配對對象脫離等待隊列，一同進入聊天狀態
	2. 若全數配對失敗，則進入隊列尾巴等候

請在不違反先來先到原則的狀況下進行平行化，也就是說，平行化之後的行爲與「一個人從頭試到尾，再換下一個人試」的行爲相同。

### send_message 相關

客戶端在「聊天中」狀態，欲傳送訊息給聊天對象時會送出：

```javascript
{
	"cmd": "send_message",
	"message": "Hello!",              // 每則訊息不超過 1024 字元
	"sequence": 4                     // 爲一個 小於 2^31 的正整數，給客戶端校驗用
}
```

回傳

```javascript
{
	"cmd": "send_message",
	"message": "Hello!",              // 與請求時一樣的 message
	"sequence": 4                     // 與請求時一樣的 sequence
}
```

而同時伺服器傳送以下訊息給聊天對象
```javascript
{
	"cmd": "receive_message",
	"message": "Hello!",              // 與請求時一樣的 message
	"sequence": 4                     // 與請求時一樣的 sequence
}
```

### quit 相關

若客戶端

1. 狀態爲配對中，但突然想放棄配對。（可能有競態關係，見[測試時的保證](#測試時的保證)）
2. 狀態爲聊天中，不想聊了。（可能有競態關係，見[測試時的保證](#測試時的保證)）

可送出

```javascript
{
	"cmd": "quit"
}
```
此時若在配對中，伺服器應該將該用戶從等待隊列中取出。

伺服器回傳給發出請求的用戶
```javascript
{
	"cmd": "quit"
}
```
若該用戶有連接對象（處於「聊天中」），發以下訊息給該對象，告訴該員對方不想跟你聊了。
```javascript
{
	"cmd": "other_side_quit"
}
```

此外，當雙方連接，但其中一方 socket 關閉，也要發給未斷線者 other\_side\_quit 的訊息

## 額外要求

由於測試時無法控制埠口是否已經被它人佔用，如果伺服器無法綁定埠口，請在執行後的一秒內，在**標準輸出**中輸出 `socket fail\n` （\n 爲換行），並且立刻結束程式。如此測試程式就會自動更換欲綁定的 IP 位址。

## 實作技巧

### 動態函式庫

請見[動態函式庫教學](dl.md)。

### 惡意程式碼
若用戶刻意上傳程式碼時，我們並沒有一個有效的方式不去實際執行此程式碼就辨識出它是惡意的，但若在伺服器主進程內執行惡意程式碼，很容易就會導致整個服務崩潰，因此建議 fork 出專門執行不可控程式碼的進程，透過監控或與之溝通來知悉此程式碼的執行結果。

而當子進程死去時，可捕捉 SIGCHLD 後，再開一個子進程，更甚者，可以使用 `signalfd` 來透過檔案描述子獲得信號，採用此法即可將檔案描述子加入 select ，使得 IO 的介面更一致。

### socket教學
請前往 [socket 教學](socket.md)

### 偵測 socket 關閉
使用 select 此一 IO 多工函式時，當所監聽的 socket 在另一端被關閉，此一 socket 描述子會變爲可讀狀態，但去讀取它時，返回值會是 0 。我們可以透過此一現象來偵測關閉。

### 分割 JSON
在 TCP 協定中，一次 send 不一定會對應到一次 recv ，因此客戶端的一次 JSON 請求未必能被一次讀出，需要撰寫邏輯來將不同份的 JSON 分開。

## 評分

共 8 分
1. (2 分) 10 人以下、篩選函式只有常數複雜度時，所有 API 回應正確
2. (2 分) 以至少兩倍於單 CPU 的速度處理篩選函式
3. (2 分) 能抵擋惡意篩選函式，不使伺服器停擺
4. (2 分) 支援 ~~1000~~ 800 人上線、兩秒內 100 條訊息傳遞（無論篩選函式的忙碌程度）


### 測試時的保證

- 客戶端所有 API 格式皆正確
- 執行繳交的伺服器程式後，會等待一秒才讓客戶端開始進行連線
- 篩選函式保證能通過編譯（12/17 新增）
- 篩選函式不會掉入無窮迴圈或無窮遞迴
- 惡意篩選函式至多只會嘗試以堆疊溢出一類的方式來使程序崩潰，不會進行更多打擊
- 客戶端的所有 socket 都會正常關閉
- 同時連線數不超過 ~~1000~~ 800 ，但歷史連線數（斷掉的也算入）可以超過 ~~1000~~ 800
- 一般使用狀況下，使用者正要從「配對中」轉爲「聊天中」，伺服器要送出 matched 指令但客戶端尚未收到的同時，客戶端有可能也會送出 quit 。但測試時保證不會發生，而兩個客戶端同時送出 quit 的情況也不會發生，測試時都會在非常確定客戶端狀態不可能有改變時，才會送出該狀態下可能會送出的 API（透過控制所有客戶端可以做到） 。


## 繳交方式

請在 repo 下建立一個 MP4 目錄 ，將 Makefile 與程式碼置於其中。

```
repo
├── MP0
│   └── ...
├── MP1
│   └── ...
├── MP2
│   └── ...
├── MP3
│   └── ...
└── MP4
    ├── Makefile
    └── other files
```
我們會在 MP4 目錄下執行 `make` ，並測試此指令生成的 inf-bonbon-server 執行檔。


## 連結整理

- [無限城](https://city-of-infinity.com)
- [Introducing JSON](https://www.json.org/json-zh.html)
- [MDN 中的 JSON 介紹](https://developer.mozilla.org/zh-TW/docs/Web/JavaScript/Reference/Global_Objects/JSON)
- [cJSON (C 語言實作)](https://github.com/DaveGamble/cJSON)
- [JSON for modern C++ (C++ 實作)](https://github.com/nlohmann/json)
- [動態函式庫教學](dl.md)。
- [socket 教學](socket.md)
