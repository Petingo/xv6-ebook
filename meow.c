#include "user.h"

void test_lseek(){
  char buf[128];
  int fd = open("zombie", O_RDONLY);

  lseek(fd, 20, SEEK_SET);
  int bytesRead = read(fd, buf, 128);
  printf(1, "%d %d\n", bytesRead, buf[32]);

  bytesRead = read(fd, buf, 128);
  printf(1, "%d %d\n", bytesRead, buf[32]);

  lseek(fd, 20, SEEK_SET);
  bytesRead = read(fd, buf, 128);
  printf(1, "%d %d\n", bytesRead, buf[32]);

}

struct iovec {
    void* iov_base; /* Starting address */
    int iov_len; /* Length in bytes */
};

void test_writev(){
  static char part1[] = "THIS IS FROM WRITEV";
  static char part2[] = "[";
  static int  part3 = 65;
  
  struct iovec iov[3];

  iov[0].iov_base = part1;
  iov[0].iov_len = strlen(part1);

  iov[1].iov_base = part2;
  iov[1].iov_len = strlen(part2);

  iov[2].iov_base = &part3;
  iov[2].iov_len = sizeof(int);

  printf(1, "iov[0]: ");
  printf(1, "%s\n", part1);
  printf(1, "iov[1]: ");
  printf(1, "%s\n", part2);
  printf(1, "iov[2]: ");
  printf(1, "%c\n", part3);
  printf(1, "\n");

  printf(1, "writev(1, iov, 3): \n");
  writev(1, iov, 3);

}
int main(int argc, char *argv[])
{
  int fdc, fdf;
  fdc = open("test.txt", O_RDONLY);
  fdf = open("meow.txt", O_RDONLY);
  char bufferc[5];
  char bufferf[20];
  read(fdc, bufferc, 5);
  int offset = 0;
  for (int i = 0; i < 2; i++)
  {
    offset += bufferc[i] - '0';
    if (i < 1)
      offset *= 10;
  }
  read(fdf, bufferf, 20);
  printf(1, "%s\n", bufferf);
  printf(1, "%d\n", offset);
  lseek(fdf, 0, SEEK_SET);
  read(fdf, bufferf, 20);
  printf(1, "%s\n", bufferf);
  exit();
}
