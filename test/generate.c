#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_NAME "answer/bin/floppy.img"
#define SECTOR_SIZE 512
#define MAX_SECTOR 2880
#define ROOT_INDEX 0xFF

typedef unsigned char byte;

#define FS_MAP_SECTOR_NUMBER 0x100
#define FS_NODE_SECTOR_NUMBER 0x101
#define FS_DATA_SECTOR_NUMBER 0x103

extern void createDirectory(byte buf[MAX_SECTOR][SECTOR_SIZE], char* dirname, byte parent_index);
extern void insertFile(byte buf[MAX_SECTOR][SECTOR_SIZE], char* filename, byte parent_index);

void test_1(byte buf[MAX_SECTOR][SECTOR_SIZE]);
void test_2(byte buf[MAX_SECTOR][SECTOR_SIZE]);
void test_3(byte buf[MAX_SECTOR][SECTOR_SIZE]);
void test_4(byte buf[MAX_SECTOR][SECTOR_SIZE]);

int main(int argc, char* argv[]) {
  FILE* image = fopen(IMAGE_NAME, "rb");

  byte buf[MAX_SECTOR][SECTOR_SIZE];

  for (int i = 0; i < MAX_SECTOR; i++) {
    fread(buf[i], SECTOR_SIZE, 1, image);
  }

  fclose(image);

  image = fopen(IMAGE_NAME, "wb");

  if (argc < 2) {
    printf("Usage: %s <test_number>\n", argv[0]);
    return 1;
  }

  int test_number = atoi(argv[1]);
  switch (test_number) {
  case 1:
    test_1(buf);
    break;
  case 2:
    test_2(buf);
    break;
  case 3:
    test_3(buf);
    break;
  case 4:
    test_4(buf);
    break;
  default:
    printf("Invalid test number\n");
    return 1;
  }

  for (int i = 0; i < MAX_SECTOR; i++) {
    fwrite(buf[i], SECTOR_SIZE, 1, image);
  }

  fclose(image);

  return 0;
}

void test_1(byte buf[MAX_SECTOR][SECTOR_SIZE]) {
  createDirectory(buf, "dir1", ROOT_INDEX);
  createDirectory(buf, "dir2", ROOT_INDEX);
  createDirectory(buf, "dir1-1", 0);
  createDirectory(buf, "dir1-2", 0);
  createDirectory(buf, "dir1-1-1", 2);
  createDirectory(buf, "dirname", 1);
  createDirectory(buf, "dirname", 3);
  createDirectory(buf, "dir3", ROOT_INDEX);
}

void test_2(byte buf[MAX_SECTOR][SECTOR_SIZE]) {
  insertFile(buf, "test/files/test_2/file-0", ROOT_INDEX);

  createDirectory(buf, "dir-1", ROOT_INDEX);
  for (int i = 2; i < 63; i++) {
    char dirname[14];
    sprintf(dirname, "dir-%d", i);
    createDirectory(buf, dirname, i - 1);
  }

  insertFile(buf, "test/files/test_2/file-63", ROOT_INDEX);
}

void test_3(byte buf[MAX_SECTOR][SECTOR_SIZE]) {
  insertFile(buf, "test/files/test_3/1024", ROOT_INDEX);
  insertFile(buf, "test/files/test_3/4096", ROOT_INDEX);

  for (int i = 0; i < 14; i++) {
    char path[100];
    sprintf(path, "test/files/test_3/8192_%d", i);
    printf("Creating %s\n", path);
    insertFile(buf, path, ROOT_INDEX);
  }
}

void test_4(byte buf[MAX_SECTOR][SECTOR_SIZE]) {
  createDirectory(buf, "dir1", ROOT_INDEX);
  createDirectory(buf, "dir2", ROOT_INDEX);

  insertFile(buf, "test/files/test_4/katanya", 0);

  createDirectory(buf, "dir3", 0);

  insertFile(buf, "test/files/test_4/bikin", 3);
  insertFile(buf, "test/files/test_4/fp", 3);

  createDirectory(buf, "dir4", 3);
  createDirectory(buf, "dir5", 0);

  insertFile(buf, "test/files/test_4/cuma", 7);
  insertFile(buf, "test/files/test_4/seminggu", 7);

  insertFile(buf, "test/files/test_4/doang", ROOT_INDEX);
}
