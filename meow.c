#include "user.h"

// structs:
struct iovec
{
    void *iov_base;
    int iov_len;
};

// functions:

// 將文字檔對應的 config 檔（包含上次離開的地方、書籤等資料）讀入
void read_config(char *filename, int *lastOffset, int *bookMarks)
{
    struct iovec iov[2];
    iov[0].iov_base = lastOffset;
    iov[0].iov_len = 4;

    iov[1].iov_base = bookMarks;
    iov[1].iov_len = 400;

    int fd = open(filename, O_RDONLY);
    readv(fd, iov, 2);
    close(fd);
}

// 離開時寫入 config
void write_config(char *filename, int *lastOffset, int *bookMarks)
{
    struct iovec iov[2];
    iov[0].iov_base = lastOffset;
    iov[0].iov_len = 4;

    iov[1].iov_base = bookMarks;
    iov[1].iov_len = 400;

    int fd = open(filename, O_WRONLY);
    writev(fd, iov, 2);
    close(fd);
}

// main program:

// window width & window height
#define winWidth 60
#define winHeight 18

// book amount
#define BOOK_AMOUNT 5

// define all states
#define EXIT 0
#define SELECT_BOOK 1
#define START_READING 2
#define READING 3
#define READING_REACH_END 4

// const variables
static char booksFileName[BOOK_AMOUNT][50] = {
    "HANSEL_AND_GRETEL.txt\0", "IRON_HANS.txt\0", "RAPUNZEL.txt\0",
    "THE_QUEEN_BEE.txt\0", "THE_SEVEN_RAVENS.txt\0"};

static char booksConfigFileName[BOOK_AMOUNT][50] = {
    "HANSEL_AND_GRETEL_config.txt\0", "IRON_HANS_config.txt\0",
    "RAPUNZEL_config.txt\0", "THE_QUEEN_BEE_config.txt\0",
    "THE_SEVEN_RAVENS_config.txt\0"};

static char booksName[BOOK_AMOUNT][50] = {"Hansel and Gretel\0", "Iron Hans\0",
                                          "Rapunzel\0", "The Queen Bee\0",
                                          "The Seven Ravens"};

// variables for controller
static int state;
static int selectedBook;
static int booksFD[BOOK_AMOUNT];
static int booksConfigFD[BOOK_AMOUNT];
static char readBuffer[winWidth * winHeight];
static char displayBuffer[winWidth * winHeight];
static int pageStartOffset;
static int offset;

// 使用者換頁時，我們會計算該頁可以容納的字元數目，並將結果存進 buffer 裡面
// 以便後續的操作
void processContent()
{
    int counter = 0;
    int contentHeight = 0;
    int i;

    pageStartOffset = offset;

    // 利用行高控制還需要讀入多少文字
    for (i = 0; i < winWidth * (winHeight - 1) - 1 && readBuffer[i] != '@' &&
                contentHeight < winHeight - 1;
         i++)
    {
        if (counter == winWidth || readBuffer[i] == '\n')
        {
            contentHeight++;
            counter = 0;
            if (contentHeight == winHeight - 1)
            {
                break;
            }
        }

        displayBuffer[i] = readBuffer[i];
        counter++;
    }

    offset += i;

    // 為簡化操做，我們在檔案結尾加入 @ 字元，當讀到 @ 表示已經讀到檔案結尾
    if (readBuffer[i] == '@')
    {
        offset = 0;
        state = READING_REACH_END;
    }

    // 補滿剩下的空行
    while (contentHeight < winHeight)
    {
        displayBuffer[i++] = '\n';
        contentHeight++;
    }

    displayBuffer[i] = '\0';
}

void moveStr(char *dest, char *source, int size)
{
    int i;
    for (i = 0; i < size && source[i] != '\0'; i++)
    {
        dest[i] = source[i];
    }
    dest[i + 1] = '\0';
}


// 用來方便輸出換行的函示
void printEmptyLine(int n)
{
    for (int i = 0; i < n; i++)
    {
        printf(1, "\n");
    }
}

// 用來渲染「選輸」的畫面
void updateSelectBookView()
{
    printf(1, "Which book do you want to read?\n");
    
    for (int i = 0; i < BOOK_AMOUNT; i++)
    {
        printf(1, " %c: %s\n", 'a' + i, booksName[i]);
    }

    for (int i = 0; i < winHeight - BOOK_AMOUNT - 2; i++)
    {
        printf(1, "\n");
    }
}

// 用來渲染「開始閱讀」的畫面
void updateStartReadingView()
{
    printEmptyLine(8);
    printf(1, "Starting Reading \"%s\" from the place you \nleft last time...",
           booksName[selectedBook]);
    printEmptyLine(8);
}

// 在使用者閱讀換頁時，渲染畫面
int updateReadingView()
{
    printf(1, "%s", displayBuffer);
    return 0;
}

// 更新畫面，會根據當前的狀態選擇不同的函數呼叫
void updateView()
{
    if (state == START_READING)
    {
        updateStartReadingView();
    }
    else if (state == SELECT_BOOK)
    {
        updateSelectBookView();
    }
    else if (state == READING || state == READING_REACH_END)
    {
        updateReadingView(displayBuffer);
    }
}

// 主程式
int main(int argc, char *argv[])
{
    for (int i = 0; i < BOOK_AMOUNT; i++)
    {
        booksFD[i] = open(booksFileName[i], O_RDONLY);
    }
    for (int i = 0; i < BOOK_AMOUNT; i++)
    {
        booksConfigFD[i] = open(booksConfigFileName[i], O_RDWR);
    }

    // 程式開始執行時會先跳到選書的畫面
    state = SELECT_BOOK;
    updateView();

    // varable use to save config
    int bookMarks[100];

    // 不斷讀取使用者的輸入，直到狀態變成「退出」
    while (state != EXIT)
    {
        // 讀輸入的部分
        char c[10] = {0};
        read(1, &c, 10);

        // 當前在選書時，輸入的內容為 a, b, c, d, e，代表要看不同的書
        if (state == SELECT_BOOK)
        {
            switch (c[0])
            {
            case 'a':
                selectedBook = 0;
                state = START_READING;
                break;
            case 'b':
                selectedBook = 1;
                state = START_READING;
                break;
            case 'c':
                selectedBook = 2;
                state = START_READING;
                break;
            case 'd':
                selectedBook = 3;
                state = START_READING;
                break;
            case 'e':
                selectedBook = 4;
                state = START_READING;
                break;
            case 'q':
                state = EXIT;
                break;
            }
            updateView();
        }

        // 開始閱讀時
        else if (state == START_READING)
        {
            // 將對應書籍的 config 載入
            read_config(booksConfigFileName[selectedBook], &offset, bookMarks);

            // 移動到上次離開的地方
            lseek(booksFD[selectedBook], offset, SEEK_SET);

            // 把文字內容讀入 buffer
            read(booksFD[selectedBook], readBuffer, winWidth * winHeight);

            // 將剛剛讀入 buffer 的文字處理（主要是要計算會顯示多少字）
            processContent(displayBuffer);

            // 將 state 設為「閱讀」
            state = READING;
            updateView();
        }

        // 閱讀
        else if (state == READING)
        {
            switch (c[0])
            {
            // enter => 翻頁
            case '\n':
                // 先用 lseek 移動到上次結束顯示的地方
                lseek(booksFD[selectedBook], offset, SEEK_SET);

                // 把文字讀進 buffer
                read(booksFD[selectedBook], readBuffer, winWidth * winHeight);

                // 處理文字
                processContent(displayBuffer);

                // 更新畫面
                updateView();

                // 如果文章已經結束，就將 state 設為「選書」，下次使用者
                // 按 enter 時就會回到選輸的畫面
                if (state == READING_REACH_END)
                {
                    state = SELECT_BOOK;
                }

                break;

            // 跳到書籤位置
            case 'j':
            {
                // 先讀入使用者要跳到哪個書籤
                int mark_number;
                mark_number = (c[2] - '0') * 10 + (c[3] - '0');

                // 將檔案的 offset 設定為書籤紀錄的位置，並用 lseek 跳過去
                offset = bookMarks[mark_number];
                lseek(booksFD[selectedBook], offset, SEEK_SET);

                // 將文字讀入 buffer
                read(booksFD[selectedBook], readBuffer, winWidth * winHeight);

                // 處理文字
                processContent(displayBuffer);

                // 更新畫面
                updateView();
            }
            break;

            // 儲存書籤
            case 'b':
            {
                // 先讀使用者要存在第幾號書籤
                int mark_number;
                mark_number = (c[2] - '0') * 10 + (c[3] - '0');

                // 把當前的 offset 作為書籤，記錄起來
                bookMarks[mark_number] = pageStartOffset;
            }
            break;

            // 離開
            case 'q':
                // 將 state 設為「選書」
                state = SELECT_BOOK;

                // 把書籤、現在的書籤位置存入 config
                offset = pageStartOffset;
                write_config(booksConfigFileName[selectedBook], &pageStartOffset,
                             bookMarks);
                updateView();
                break;
            }
        }
        else if (state == READING_REACH_END)
        {
            updateView();
        }
    }

    // 離開
    printf(1, "Good Bye!\n");

    for (int i = 0; i < BOOK_AMOUNT; i++)
    {
        close(booksFD[i]);
        close(booksConfigFD[i]);
    }
    exit();
}

