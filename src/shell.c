#include "shell.h"
#include "filesystem.h"

// TODO: 10. Complete shell function
void shell() {
  char buf[64];
  char path[128];

  byte cwd = FS_NODE_P_ROOT;

  while (true) {
    printString("MengOS:");
    printCWD(path, cwd);
    printString("$ ");
    readString(buf);

    // parse buf by space first

    if (strcmp(buf, "ls")) {
      // add ls utilities here
    }

    // add more commands here

    else {
      printString("Invalid command\n");
    }
  }
}

// TODO: 4. Implement printCWD function
void printCWD(char path[128], byte cwd) {}

// TODO: 5. Implement ls function
void ls(char* dirname) {}

// TODO: 6. Implement mv function
void mv(char* src, char* dst) {}

// TODO: 7. Implement cp function
void cp(char* filename) {}

// TODO: 8. Implement cat function
void cat(char* filename) {}

// TODO: 9. Implement mkdir function
void mkdir(char* dirname) {}
