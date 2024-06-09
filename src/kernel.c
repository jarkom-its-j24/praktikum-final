#include "shell.h"
#include "kernel.h"
#include "filesystem.h"

void main() {
  fsInit();
  clearScreen();
  shell();
}

void printString(char* str) {
  while (*str != '\0') {
    if (*str == '\n') interrupt(0x10, 0xE << 8 | '\r', 0, 0, 0);
    interrupt(0x10, 0xE << 8 | *str, 0, 0, 0);
    str++;
  }
}

void readString(char* buf) {
  int i = 0;
  char c = 0;

  while (c != '\r') {
    c = interrupt(0x16, 0, 0, 0, 0);
    if (c == '\b') {
      if (i > 0) {
        printString("\b \b");
        i--;
      }
    }
    else {
      buf[i++] = c;
      interrupt(0x10, 0xE << 8 | c, 0, 0, 0);
    }
  }

  buf[i - 1] = '\0';
  printString("\n");
}

void clearScreen() {
  int j, i = 0;

  while (i < 25) {
    j = 0;
    while (j < 80) {
      putInMemory(0xB000, 0x8000 + 2 * (i * 80 + j), 0x0);
      putInMemory(0xB000, 0x8001 + 2 * (i * 80 + j), 0x7);
      j++;
    }
    i++;
  }

  interrupt(0x10, 0x2 << 8, 0, 0, 0);
}

void readSector(byte* buf, int sector) {
  int ah = 0x02;                    // read sector service number
  int al = 0x01;                    // number of sectors to read
  int ch = div(sector, 36);         // cylinder number
  int cl = mod(sector, 18) + 1;     // sector number
  int dh = mod(div(sector, 18), 2); // head number
  int dl = 0x00;                    // drive number

  interrupt(
    0x13,
    ah << 8 | al,
    buf,
    ch << 8 | cl,
    dh << 8 | dl
  );
}

// TODO: 1. Implement writeSector function
void writeSector(byte* buf, int sector) {}
