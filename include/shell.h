#ifndef __SHELL_H__
#define __SHELL_H__

#include "std_type.h"

void shell();
void printCWD(char path[128], byte cwd);

void ls(char* dirname);
void mv(char* src, char* dst);
void cp(char* filename);
void cat(char* filename);
void mkdir(char* dirname);

#endif // __SHELL_H__
