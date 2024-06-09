#ifndef __SHELL_H__
#define __SHELL_H__

#include "std_type.h"

void shell();
void printCWD(byte cwd);
void parseCommand(char* buf, char* cmd, char arg[2][64]);

void cd(byte* cwd, char* dirname);
void ls(byte cwd, char* dirname);
void mv(byte cwd, char* src, char* dst);
void cp(byte cwd, char* src, char* dst);
void cat(byte cwd, char* filename);
void mkdir(byte cwd, char* dirname);

#endif // __SHELL_H__
