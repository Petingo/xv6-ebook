#include "user.h"

// structs:
struct iovec{
  void* iov_base;
  int iov_len;
};

// functions:
void read_config(char* filename, int* lastOffset, int* bookMarks){
  struct iovec iov[2];
  iov[0].iov_base = lastOffset;
  iov[0].iov_len = 4;

  iov[1].iov_base = bookMarks;
  iov[1].iov_len = 400;

  int fd = open(filename, O_RDONLY);
  readv(fd, iov, 2);
  close(fd);
}

void write_config(char* filename, int* lastOffset, int* bookMarks){
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

#define winWidth 60
#define winHeight 18

#define BOOK_AMOUNT 5

#define EXIT 0
#define SELECT_BOOK 1
#define START_READING 2
#define READING 3
#define READING_REACH_END 4

static int state;
static int selectedBook;
static int booksFD[BOOK_AMOUNT];
static int booksConfigFD[BOOK_AMOUNT];
static char readBuffer[winWidth * winHeight];
static char displayBuffer[winWidth * winHeight];
static int pageStartOffset;
static int offset;

static char booksFileName[BOOK_AMOUNT][50] = {
	"HANSEL_AND_GRETEL.txt\0",
	"IRON_HANS.txt\0",
	"RAPUNZEL.txt\0",
	"THE_QUEEN_BEE.txt\0",
	"THE_SEVEN_RAVENS.txt\0"
};

static char booksConfigFileName[BOOK_AMOUNT][50] = {
  	"HANSEL_AND_GRETEL_config.txt\0",
	"IRON_HANS_config.txt\0",
	"RAPUNZEL_config.txt\0",
	"THE_QUEEN_BEE_config.txt\0",
	"THE_SEVEN_RAVENS_config.txt\0"
};

static char booksName[BOOK_AMOUNT][50] = {
	"Hansel and Gretel\0",
	"Iron Hans\0",
	"Rapunzel\0",
	"The Queen Bee\0",
	"The Seven Ravens"
};

void processContent(){
  int counter = 0;
  int contentHeight = 0;
  int i;

  pageStartOffset = offset;
  
  for(i = 0; i < winWidth * (winHeight - 1) - 1 && readBuffer[i] != '@' && contentHeight < winHeight - 1 ; i++){
    if(counter == winWidth || readBuffer[i] == '\n'){
      contentHeight++;
      counter = 0;
      if(contentHeight == winHeight - 1){
        break;
      }
    }

    displayBuffer[i] = readBuffer[i];
    counter++;
  }
  
  offset += i;

  if(readBuffer[i] == '@'){
    offset = 0;
    state = READING_REACH_END;
  }

  while(contentHeight < winHeight){
    displayBuffer[i++] = '\n';
    contentHeight++;
  }
  
  displayBuffer[i] = '\0';
}

void printEmptyLine(int n){
  for(int i = 0 ; i < n ; i++){
    printf(1, "\n");
  }
}

void updateSelectBookView(){
  printf(1, "Which book do you want to read?\n");
  for(int i = 0 ; i < BOOK_AMOUNT ; i++){
    printf(1, " %c: %s\n", 'a'+i, booksName[i]);
  }
  for(int i = 0 ; i < winHeight - BOOK_AMOUNT - 2 ; i++){
    printf(1, "\n");
  }
}

void updateStartReadingView(){
  printEmptyLine(8);
  printf(1, "Starting Reading \"%s\" from the place you \nleft last time...", booksName[selectedBook]);
  printEmptyLine(8);
}

int updateReadingView(){
  printf(1, "%s", displayBuffer);
  return 0;
}

void moveStr(char *dest, char *source, int size){
  int i;
  for(i = 0 ; i < size && source[i] != '\0'; i++){
    dest[i] = source[i];
  }
  dest[i+1] = '\0';
}

void updateView(){
  if (state == START_READING){
    updateStartReadingView();
  }
  else if (state == SELECT_BOOK) {
    updateSelectBookView();
  }
  else if (state == READING || state == READING_REACH_END){
    updateReadingView(displayBuffer);
  }
}

int main(int argc, char *argv[]){

  for(int i = 0 ; i < BOOK_AMOUNT ; i++){
    booksFD[i] = open(booksFileName[i], O_RDONLY);
  }
  for(int i = 0 ; i < BOOK_AMOUNT ; i++){
    booksConfigFD[i] = open(booksConfigFileName[i],O_RDWR);
  }

  state = SELECT_BOOK;
  updateView();
  // varable use to save config
  int bookMarks[100];

  while(state != EXIT){
    char c[10] = {0};
    read(1, &c, 10);
    //printf(1, "input = %d %d %d %d, statet = %d", c[0], c[1], c[2], c[3], state);
    if(state == SELECT_BOOK){
      switch(c[0]){
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
    else if(state == START_READING){
      // Use config to set offset
      read_config(booksConfigFileName[selectedBook], &offset, bookMarks);
      //printf(1, "start_reading offset: %d\n", offset);
      
      lseek(booksFD[selectedBook], offset, SEEK_SET);
      read(booksFD[selectedBook], readBuffer, winWidth * winHeight);

      state = READING;
      processContent(displayBuffer);
      updateView();
    }
    else if(state == READING){
      switch(c[0]){
        case '\n':
          lseek(booksFD[selectedBook], offset, SEEK_SET);
          read(booksFD[selectedBook], readBuffer, winWidth * winHeight);
          processContent(displayBuffer);

          updateView();

          if(state == READING_REACH_END){
            state = SELECT_BOOK;
          }

          break;

        case 'j':
          {
          int mark_number;
          mark_number = (c[2] - '0') * 10 + (c[3] - '0');
          offset = bookMarks[mark_number];
          lseek(booksFD[selectedBook], offset, SEEK_SET);
          read(booksFD[selectedBook], readBuffer, winWidth * winHeight);
          processContent(displayBuffer);

          updateView();
          }
          break;

        case 'b':
          {
          // book mark
          int mark_number;
          mark_number = (c[2] - '0') * 10 + (c[3] - '0');
          bookMarks[mark_number] = pageStartOffset;
          }
          break;

        case 'q':
          state = SELECT_BOOK;
          // printf(1, "q offset: %d, pageStart: %d\n", offset, pageStartOffset);
          offset = pageStartOffset;
          write_config(booksConfigFileName[selectedBook], &pageStartOffset, bookMarks);
          updateView();
          break;
      }  
    }
    else if(state == READING_REACH_END){
      updateView();
    }
  }
  printf(1, "Good Bye!\n");

  for(int i = 0 ; i < BOOK_AMOUNT ; i++){
    close(booksFD[i]);
    close(booksConfigFD[i]);
  }
  exit();
}



