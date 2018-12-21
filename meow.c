#include "user.h"

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
static int offset;

static char booksFileName[BOOK_AMOUNT][50] = {
	"HANSEL_AND_GRETEL.txt\0",
	"IRON_HANS.txt\0",
	"RAPUNZEL.txt\0",
	"THE_QUEEN_BEE.txt\0",
	"THE_SEVEN_RAVENS.txt\0"
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

  for(i = 0; i < winWidth * winHeight && readBuffer[i] != '@' && contentHeight < winHeight - 1 ; i++){
    if(counter == winWidth || readBuffer[i] == '\n'){
      contentHeight++;
      counter = 0;
    }

    displayBuffer[i] = readBuffer[i];
    counter++;
  }
  
  offset += i;

  if(readBuffer[i] == '@'){
    offset = 0;
    state = READING_REACH_END;
  }

  while(contentHeight < winHeight - 1 ){
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
  printf(1, "%s\n", booksFileName[0]);
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
  } else if (state == SELECT_BOOK) {
    updateSelectBookView();
  } else if (state == READING){
    updateReadingView(readBuffer);
  }
}

int main(int argc, char *argv[]){

  for(int i = 0 ; i < BOOK_AMOUNT ; i++){
    booksFD[i] = open(booksFileName[i], O_RDONLY);
  }
  for(int i = 0 ; i < BOOK_AMOUNT ; i++){
    booksConfigFD[i] = 0;
  }

  state = SELECT_BOOK;
  updateView();

  while(state != EXIT){
    char c[10];
    read(1, &c, 10);
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
    }
    else if(state == START_READING){
      // Use config to set offset
      offset = 0;
      state = READING;
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
          // jump to book mark
          break;

        case 'b':
          // book mark
          break;

        case 'q':
          state = SELECT_BOOK;
          updateView();
          break;
      }
      
    }
  }
  printf(1, "Good Bye!\n");
  exit();
}



