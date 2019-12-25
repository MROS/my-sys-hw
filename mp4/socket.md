# 簡易socket入門

## 簡介

在 MP3 作業我們使用過 named pipe （或稱 [FIFO](https://dictionary.cambridge.org/pronunciation/english/fifo) ）當做進程間互相交換資料的管道，同樣的， socket 和 FIFO 也是衆多 UNIX 的進程間傳輸機制之一，有別於 FIFO 只能單向， socket 支援雙向傳輸，且 socket 可以透過網路傳輸資料而不僅限於單一作業系統。

Linux 系統的 socket 可以支援很多界面，有透過網路傳輸的有 [TCP 或 UDP](http://opencourse.ncyu.edu.tw/ncyu/file.php/15/week10/TCP%E8%88%87UDP.pdf) 協定的 socket ，也有只能單機使用的 [UNIX domain socket](https://en.wikipedia.org/wiki/Unix_domain_socket) 。爲了方便說明，本節的 socket 都是指 TCP socket 。

## 作業流程

socket 常應用在網頁服務，以你使用瀏覽器上網 Google 爲例， Google 伺服器總是會開着一個 *listen* socket 等待新的連線，而客戶端瀏覽器則利用 *connect* socket 主動連線到 Google 伺服器的 socket ，在伺服器的 socket 接受（ *accept* ）你的連線之後方可開始資料傳輸，每次新連線不斷重複這個過程。

有注意到 *connect* 、 *listen* 、 *accept* 這些關鍵字標記斜體？他們是使用 C 語言實作 socket 時用到的 syscall 名稱`connect()`、`listen()`、`accept()`，客戶和伺服端兩邊各調用不同的syscall，寫法不盡相同，我們可以先看下面簡單範例。

## 客戶端socket
socket在C語言中是一個檔案描述子（ file descriptor ），使用方式和以前作業的檔案描述子的操作方式是幾乎一樣的，差在是這裏用`recv()`和`send()`代替`read()`和`write()`傳輸資料。

客戶端操作流程大致分成
1. 創建 socket 檔案描述子，也就是範例的 sockfd 。
2. 設定 struct sockaddr_in 結構來儲存連線目標的網路位置。
3. 呼叫`connect()`令socket主動連線到伺服器。
4. 使用`recv()`、`send()`進行傳輸。
5. 呼叫`close()`終止連線。

```c
// 宣告 socket 檔案描述子， AF_INET 、 SOCK_STREAM 代表使用透過網路的 TCP 協定
int sockfd = socket(AF_INET , SOCK_STREAM , 0);
assert(sockfd >= 0);

// 利用 struct sockaddr_in 設定連線目標的 IP 位置
struct sockaddr_in addr;
/* 設定 addr 變數
   內容填寫連線目標的IP位置
   細節請見後面段落
   ...                 */

// 使用 connect() 連線到伺服器
int retval = connect(sockfd, (struct sockaddr*) &addr, sizeof(addr));
assert(!retval);

// 進行讀寫
ssize_t sz;
sz = recv(sockfd, buffer, buffer_size, 0); // 接收資料
sz = send(sockfd, buffer, buffer_size, 0); // 輸出資料

// 結束連線
close(sockfd);
```

## 伺服端socket

伺服端 socket 和客戶端使用方式不同，並不直接用於資料傳輸，反之，它只能用在`accept()`來接受新連線，`accept()`返回是新的檔案描述子，你再用這新的描述子和客戶端交換資料。例如假定有 1000 個客戶端連線進來，就得呼叫`accept()`共 1000 次，之後得到共 1000 個描述子，各個可以和一個客戶端交換資料。

伺服端操作流程大致分成
1. 創建 socket 檔案描述子，也就是範例的 sockfd 。
2. 設定 struct sockaddr_in 結構來儲存伺服器"自己"的網路位置，並用`bind()`*綁定*。
3. 呼叫`listen()`告知作業系統這是一個伺服 socket 。
4. 呼叫`accept()`等待新連線，返回後會得到新的 fd 。
5. 在這新的fd呼叫`recv()`、`send()`和客戶端傳輸資料。
6. 在新 fd 上呼叫`close()`關閉客戶端連線，如果要再開新連線可以重複 4 、 5 、 6 步驟。
7. 結束程式前呼叫`close()`關閉 sockfd 。

```c
// 宣告 socket 檔案描述子
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
assert(sockfd >= 0);

// 利用 struct sockaddr_in 設定伺服器"自己"的位置
struct sockaddr_in server_addr;
/* 設定 server_addr 變數
   內容填寫伺服器自己的IP位置
   細節請見後面段落
   ...                  */

// 使用 bind() 將伺服socket"綁定"到特定 IP 位置
int retval = bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr));
assert(!retval);

// 呼叫 listen() 告知作業系統這是一個伺服socket
retval = listen(sockfd, 5);
assert(!retval);

while (1)
{
    // 呼叫 accept() 會 block 住程式直到有新連線進來，回傳值代表新連線的描述子
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_fd = accept(sockfd, (struct sockaddr*) &client_addr, &addrlen);
    assert(client_fd >= 0);

    /* 注意實作上細節，呼叫 accept() 時 addrlen 必須是 client_addr 結構的長度
       accept() 回傳之後會將客戶端的 IP 位置填到 client_addr
       並將新的 client_addr 結構長度填寫到 addrlen                        */

    // 和客戶端交換資料
    ssize_t sz;
    sz = recv(client_fd, buffer, buffer_size, 0); // 接收資料
    sz = send(client_fd, buffer, buffer_size, 0); // 輸出資料

    // 用完時關閉檔案描述子
    close(client_fd);
}

// 結束程式前記得關閉 sockfd
close(sockfd);
```

## 網路定址

在範例程式中，每次使用 socket 得使用`struct sockaddr_in`結構來填寫網路位置，所謂網路位置由兩個 IP 位置、 TCP 埠口兩個元素構成。

**IP位置**

IP 位置就像家門口的門牌地址，藉由這位置電腦能透過路由器和交換器連線到伺服器。回到上面的例子，你在瀏覽器輸入網址 www.google.com 後，瀏覽器先用網址查詢 Google 伺服器的 IP 位置，再用查詢到的 IP 連到 Google 網頁。

同樣的，我可以在終端機用dig指令手工查詢 www.google.com 的 IP 位置，其輸出 ANSWER SECTION 中，記載 216.58.200.36 便是 Google 伺服器的 IP 位置，可以試試看在瀏覽器網址直接輸入 216.58.200.36 也能連 Google 網頁。

```sh
~$ dig www.google.com
...略...
;; ANSWER SECTION:
www.google.com.		183	IN	A	216.58.200.36
...略...
```

有的同學會注意到 IP 位置通常會用 xxx.xxx.xxx.xxx 格式標記，總共四碼 0 到 255 之間用逗點分割數字，這位置在記憶體中可以用剛好四個 byte 表示（爲什麼？）。

**TCP埠口**

網路上的伺服器除了有自己的 IP 位置之外，還可以提供網頁、 FTP 、 SSH 、 telnet 等服務。這些服務又可以用 TCP 埠口來區隔，慣例上 80 號埠口表示網頁服務、 22 號埠口代表 SSH 。當你在瀏覽器輸入 www.google.com 網址時，會**默認**爲你是連線到 Google 伺服器的 80 號埠口，當然，也有網頁不是放在 80 埠口的例外，比如在 MP3 作業的競賽網址[wtf.csie.org:8000](http://wtf.csie.org:8000)，其冒號後面的 8000 告知瀏覽器連線到 wtf.csie.org 伺服器的 8000 號埠口。

## 實作客戶端定址

爲了把連線目標告知客戶端 socket ，我們建立`struct sockaddr_in`結構，在裏面填寫位置類型、 IP 位置、 TCP 埠口三個欄位。在這部分我們令客戶端連線到特殊的 IP 位置 127.0.0.1 ，這是 IP 協定保留的特殊位置，用於代表本機。

```c
/* 創建socket
   ...
   ...       */

// 設定連線目標的 IP 位置和埠口
struct sockaddr_in addr;
memset(&addr, 0, sizeof(addr)); // 清零初始化，不可省略
addr.sin_family = PF_INET;      // 表示位置類型是網際網路位置
addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 連線目標 IP 是 127.0.0.1
addr.sin_port = htons(44444);   // 連線目標的 TCP 埠口是 44444

// 開始連線
int retval = connect(sockfd, (struct sockaddr*) &addr, sizeof(addr));
```

## 實作伺服端定址

伺服端實作方式和客戶端差不多，但是用途不同，在伺服程式上我們設定socket的位置的作用是爲了*綁定*，*綁定*的效果是伺服器可決定接受哪些外來連線。這裏我們使用特殊的位置 INADDR\_ANY ，表示伺服 socket 接受*所有*的外來 TCP 連線。 INADDR\_ANY 也可以用特殊 IP 位置 0.0.0.0 替代。

```c
/* 創建 socket
   ...
   ...       */

//
struct sockaddr_in server_addr;
memset(&server_addr, 0, sizeof(server_addr)); // 清零初始化，不可省略
server_addr.sin_family = PF_INET;              // 位置類型是網際網路位置
server_addr.sin_addr.s_addr = INADDR_ANY;      // INADDR_ANY 是特殊的IP位置，表示接受所有外來的連線
server_addr.sin_port = htons(44444);           // 在 44444 號 TCP 埠口監聽新連線

// 綁定位置
bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)); // 綁定 sockfd 的位置
```

## 使用`select(2)`

socket在C語言是檔案描述子，同樣可以用在`select(2)`界面，除了可以監聽是否有有新資料抵達或可寫入之外，在伺服端 socket 上還可以用來檢查是否有新連線。

```c
#define MAX_FD 1025

// 宣告 socket 檔案描述子
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
assert(sockfd >= 0);

/* 設定伺服器位置
   呼叫 bind() 綁定
   呼叫 listen()
   ...略...     */

// 宣告 select() 使用的資料結構
fd_set readset;
fd_set working_readset;
FD_ZERO(&readset);

// 將 socket 檔案描述子放進 readset
FD_SET(sockfd, &readset);

while (1)
{
    memcpy(&working_readset, &readset, sizeof(fd_set));
    retval = select(MAX_FD, &working_readset, NULL, NULL, NULL);

    if (retval < 0) // 發生錯誤
    {
        perror("select() went wrong");
        exit(errno);
    }

    if (retval == 0) // 排除沒有事件的情形
        continue;

    for (int fd = 0; fd < MAX_FD; fd += 1) // 用迴圈列舉描述子
    {
        // 排除沒有事件的描述子
        if (!FD_ISSET(fd, &working_readset))
            continue;

        // 分成兩個情形：接受新連線用的 socket 和資料傳輸用的 socket
        if (fd == sockfd)
        {
            // sockfd 有事件，表示有新連線
            struct sockaddr_in client_addr;
            socklen_t addrlen = sizeof(client_addr);
            int client_fd = accept(fd, (struct sockaddr*) &client_addr, &addrlen);
             if (client_fd >= 0)
                FD_SET(client_fd, &readset); // 加入新創的描述子，用於和客戶端連線
        }
        else
        {
            // 這裏的描述子來自 accept() 回傳值，用於和客戶端連線
            ssize_t sz;
            sz = recv(fd, buffer, buffer_size, 0); // 接收資料

            if (sz == 0) // recv() 回傳值爲零表示客戶端已關閉連線
            {
                // 關閉描述子並從 readset 移除
                close(fd);
                FD_CLR(fd, &readset);
            }
            else if (sz < 0) // 發生錯誤
            {
                /* 進行錯誤處理
                   ...略...  */
            }
            else // sz > 0，表示有新資料讀入
            {
                /* 進行資料處理
                   ...略...   */
            }
        }
    }
}

// 結束程式前記得關閉 sockfd
close(sockfd);
```

## 測試工具

### netcat / nc

netcat 號稱網路連線工具的「瑞士刀」，可以輕易建立伺服或客戶端 TCP 連線。
```sh
# 在 55555 埠等待新連線，並將讀到的資料寫如 target_file
nc -l -p 55555 > target_file

# 另一個終端機上，連到本機的 55555 埠，並送出 source_file 的內容
nc 127.0.0.1 55555 < source_file
```

甚至可以用 nc 連線到網頁伺服器。
```sh
nc www.google.com.tw 80 <<EOF
GET / HTTP/1.1
Host: www.google.com.tw

EOF
```

### nmap

nmap 可以掃描伺服器有開啓哪些 TCP 埠口，例如你可以測試 Google 的伺服器，會得到 80 和 443兩個埠口。
```sh
~$ nmap www.google.com
Starting Nmap 7.60 ( https://nmap.org ) at 2017-12-06 00:13 CST
Nmap scan report for www.google.com (216.239.38.120)
Host is up (0.020s latency).
Other addresses for www.google.com (not scanned): 2404:6800:4008:801::2004
rDNS record for 216.239.38.120: any-in-2678.1e100.net
Not shown: 998 filtered ports
PORT    STATE SERVICE
80/tcp  open  http
443/tcp open  https

Nmap done: 1 IP address (1 host up) scanned in 4.93 seconds
```

## 參考資料

* [TCP Socket Programming 學習筆記](http://zake7749.github.io/2015/03/17/SocketProgramming/)
* [維基百科常見TCP/UDP埠口列表](https://zh.wikipedia.org/wiki/TCP/UDP%E7%AB%AF%E5%8F%A3%E5%88%97%E8%A1%A8)
