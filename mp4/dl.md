# 動態函式庫簡介

## 先談靜態函式庫

打從各位開始學 C 語言入行，除了學習各種資料結構、設計演算法之外，能不能熟悉 C 標準函式庫也是很重要技能樹，即使是最簡單的 hello world 程式中，我們肯定用函數庫中寫好的`printf(3)`函數，而不會自幹一個列印函數。可以說幾乎大部分的程式需要借寫好的`printf(3)`或者標準函式庫中其他的工具。

談到這點同學可以思考一個問題， C 標準函式庫作爲天下最普及的函數庫之一，如果系統中有 1000 個進程都需要調用`printf(3)`，我們真的需要把函數`printf(3)`複製 1000 份分散塞到這些函數裏面？顯然這有不少改進之處，一來我們複製好幾個一模一樣的函數，會浪費不少記憶體，二來如果`printf(3)`的實作有更改，這 1000 個程式就得重新編譯，也增加維護上的負擔。然而，這種函式庫的確存在，被稱爲靜態函數庫（static library），當你使用靜態函式庫時，編譯器會自動複製一份函式庫副本到你的程式裏面。

實務上 Linux 的靜態函式庫會包裝成 .a 檔案，使用時作爲編譯器輸入。
```sh
gcc main.c /usr/lib/libcrypt.a -o main
```

## 爲何需要動態函式庫？

爲了解決上面的問題，動態函數庫便橫空出世，其理念可以從原文 shared library 看出來，既然 C 標準函數庫都長一樣，不需要真的複製 1000 份給這些程式，我們只要*分享*它就行了，但是*分享*這件事情要如何做到？作業系統使用聰明的解決方式，也就是利用虛擬記憶體（virtual memory）的映射機制，作業系統先把標準函式庫本尊 `/usr/lib/libc.so` 在實體記憶體（physical）中找個位置放上去，然後讓各個使用到標準函式庫的進程的虛擬記憶體中，取一部分映射過去。

我們複習一下課堂講過的進程記憶體空間，下圖右邊是簡化的記憶體區塊圖，注意中間有一段 shared libraries 區塊，便是放置動態函式庫的記憶體映射。左邊是虛擬記憶體各個分頁（page）映射到實體記憶體和硬碟 swap space 的示意圖。

![虛擬記憶體](http://slideplayer.com/slide/8530558/26/images/3/Virtual+address+space+Virtual+memory+Physical+memory.jpg)

*(取自 slideplayer.com)*

我們可以用另一種關係圖來表示動態和靜態函式庫的關係，左邊每個進程的記憶體都有一個靜態函式庫副本，而右邊每個進程使用映射到同個動態函式庫實體。

![動態和靜態記憶體示意圖](http://www.bogotobogo.com/cplusplus/images/libraries/static_vs_dynamic_1.png)

*(取自bogotobogo.com)*

Linux 的動態函式庫會封裝成 .so 檔案，通常放在 `/usr/lib` 或 `/lib` 下。而 Mac 作業系統則是 .dylib 檔案，通常也在 `/usr/lib`。 Windows 的則是 .dll 檔案。

## 使用動態函式庫

動態函式庫的使用方式非常簡單，比如我們有個只有使用 C 標準函式庫的 hello world 程式，我們不需要更動任何編譯參數。
```sh
gcc -o helloworld helloworld.c
./helloworld # 執行時自動載入動態函式庫 /usr/lib/libc.so
```

然而使用其他的動態函式庫就得調整編譯參數，例如我們寫一個使用 OpenSSL 函式庫來計算 MD5 雜湊的程式。
```sh
gcc -o md5hash -lcrpyto md5hash.c # 載入 libcrypto.so 函式庫
./md5hash # 執行時自動載入 /usr/lib/libcrypto.so
```

上面提到的方式都是編譯時列舉動態函式庫，在執行時期（runtime）就會自動載入。

## 手工載入動態函式庫

然而，如果有種情況是我們編譯時不知道動態函式庫檔案的位置，而需要在執行時才手工載入？這種情況很常見在**外掛程式**系統，例如這次作業中，我們會把各個使用者提供的篩選函數包成各個 .so 動態函式庫，然後伺服器使用時再載入。

舉例來說，我們寫了一個外掛程式`mul.c`用來執行乘法，函數定義如下。
```c
int multiply(int a, int b)
{
    return a * b;
}
```

使用編譯器建立動態函式庫`mul.so`，當然`mul.so`不像是我們之前學過的一般程式，它沒有 main 函數，也不是執行檔。
```c
gcc -fPIC -shared -o mul.so mul.c
```

接着我們寫一個主程式`main.c`，執行的時候載入`mul.so`動態函式庫，並執行其中的`multiply()`函數。流程大致如下
1. 呼叫`dlopen(3)`取得 handle 。
2. 在 handle 上呼叫 `dlsym(3)` 取得 `multiply()` 函數的function pointer。
3. 呼叫`multiply()`。
4. 最後使用`dlclose()`關閉 handle 。

```c
#include <stdio.h>
#include <dlfcn.h> // 載入標頭檔
#include <assert.h>

int main()
{
    // 建立動態函式庫的 handle
    void* handle = dlopen("./mul.so", RTLD_LAZY);
    assert(NULL != handle);

    // 載入 multiply 函數
    dlerror();
    int (*mul_fn)(int, int) = (int (*)(int, int)) dlsym(handle, "multiply");

    /* int (*mul_fn)(int, int) 表示變數 mul_fn 是一個參數爲
       (int, int) 、而回傳值是 int 的function pointer。
       dlsym 回傳值則轉換成成 int (*)(int, int) 型別，
       表示是function pointer。 */

    // 若找不到函數進行錯誤處理
    const char *dlsym_error = dlerror();
    if (dlsym_error)
    {
        fprintf(stderr, "Cannot load symbol 'multiple': %s\n", dlsym_error);
        dlclose(handle);
        return 1;
    }

    // 使用動態載入的函數
    int result = mul_fn(2, 3);
    printf("%d\n", result); // 6

    // 最後記得關閉 handle
    dlclose(handle);

    return 0;
}
```

爲了能使用 `dlopen()` 系列的函數，編譯主程式時得加上`-ldl`參數，告知編譯器使用 libdl 函數庫。
```sh
gcc main.c -ldl -o main
./main # prints 6
```
