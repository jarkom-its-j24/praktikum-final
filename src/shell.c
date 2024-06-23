#include "shell.h"
#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

void shell() {
  char buf[64];
  char cmd[64];
  char arg[2][64];

  byte cwd = FS_NODE_P_ROOT;

  while (true) {
    printString("MengOS:");
    printCWD(cwd);
    printString("$ ");
    readString(buf);
    parseCommand(buf, cmd, arg);

    if (strcmp(cmd, "cd")) cd(&cwd, arg[0]);
    else if (strcmp(cmd, "ls")) ls(cwd, arg[0]);
    else if (strcmp(cmd, "mv")) mv(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cp")) cp(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cat")) cat(cwd, arg[0]);
    else if (strcmp(cmd, "mkdir")) mkdir(cwd, arg[0]);
    else if (strcmp(cmd, "clear")) clearScreen();
    else printString("Invalid command\n");
  }
}

// TODO: 4. Implement printCWD function
void printCWD(byte cwd) {}
struct node_fs node_fs_buf;
    byte path[FS_MAX_NODE];
    int path_len = 0;
    int i;

    readNodeSectors(&node_fs_buf);

    while (cwd != FS_NODE_P_ROOT && path_len < FS_MAX_NODE) {
        path[path_len++] = cwd;
        cwd = node_fs_buf.nodes[cwd].parent_index;
    }

    if (path_len == 0) {
        printString("/");
    } else {
        for (i = path_len - 1; i >= 0; --i) {
            if (i != path_len - 1) {
                printString("/");
            }
            printString(node_fs_buf.nodes[path[i]].node_name);
        }
    }
}

// TODO: 5. Implement parseCommand function
void parseCommand(char* buf, char* cmd, char arg[2][64]) {}
int i = 0;
    int j = 0;
    int arg_index = 0;

    clear(cmd, 64);
    clear(arg[0], 64);
    clear(arg[1], 64);

    while (buf[i] == ' ') {
        i++;
    }

    while (buf[i] != ' ' && buf[i] != '\0') {
        cmd[j++] = buf[i++];
    }
    cmd[j] = '\0';

    while (buf[i] == ' ') {
        i++;
    }

    while (buf[i] != '\0' && arg_index < 2) {
        j = 0;
        while (buf[i] != ' ' && buf[i] != '\0') {
            arg[arg_index][j++] = buf[i++];
        }
        arg[arg_index][j] = '\0';

        while (buf[i] == ' ') {
            i++;
        }

        arg_index++;
    }
}

// TODO: 6. Implement cd function
void cd(byte* cwd, char* dirname) {}
  struct node_fs node_fs_buf;
    int i;

    readNodeSectors(&node_fs_buf);

    if (strcmp(dirname, "/") == 0) {
        *cwd = FS_NODE_P_ROOT;
        return;
    }

    if (strcmp(dirname, "..") == 0) {
        if (*cwd != FS_NODE_P_ROOT) {
            *cwd = node_fs_buf.nodes[*cwd].parent_index;
        } else {
            printString("Already at root directory\n");
        }
        return;
    }

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == *cwd &&
            strcmp(node_fs_buf.nodes[i].node_name, dirname) == 0) {

            if (node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR) {
                *cwd = i;
                return;
            } else {
                printString("Not a directory\n");
                return;
            }
        }
    }

    printString("Directory not found\n");
}

// TODO: 7. Implement ls function
void ls(byte cwd, char* dirname) {}
    struct node_fs node_fs_buf;
    int i;
    byte target_dir = cwd;

    readNodeSectors(&node_fs_buf);

    if (dirname != 0 && strlen(dirname) > 0 && strcmp(dirname, ".") != 0) {
        for (i = 0; i < FS_MAX_NODE; i++) {
            if (node_fs_buf.nodes[i].parent_index == cwd &&
                strcmp(node_fs_buf.nodes[i].node_name, dirname) == 0) {
                if (node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR) {
                    target_dir = i;
                    break;
                } else {
                    printString("Not a directory\n");
                    return;
                }
            }
        }

        if (i == FS_MAX_NODE) {
            printString("Directory not found\n");
            return;
        }
    }

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == target_dir && node_fs_buf.nodes[i].node_name[0] != '\0') {
            printString(node_fs_buf.nodes[i].node_name);
            printString("\t");
        }
    }

    printString("\n");
}

void markSectorUnused(struct node_fs* node_fs_buf, struct data_fs* data_fs_buf, struct map_fs* map_fs_buf, int node_index) {
    int data_index;
    int i;

    data_index = node_fs_buf->nodes[node_index].data_index;

    for (i = 0; i < 16; i++) {
        if (data_fs_buf->datas[data_index].sectors[i] != 0x00) {
            map_fs_buf->is_used[data_fs_buf->datas[data_index].sectors[i]] = false;
        } else {
            break;
        }
    }

    node_fs_buf->nodes[node_index].node_name[0] = '\0';
    node_fs_buf->nodes[node_index].parent_index = 0x00;
    node_fs_buf->nodes[node_index].data_index = 0x00;

    for (i = 0; i < 16; i++) {
        data_fs_buf->datas[data_index].sectors[i] = 0x00;
    }
}

// TODO: 8. Implement mv function
void mv(byte cwd, char* src, char* dst) {}
 cp(cwd, src, dst);
    rm(cwd, src);
}

// TODO: 9. Implement cp function
void cp(byte cwd, char* src, char* dst) {}
struct file_metadata src_metadata;
    struct file_metadata dst_metadata;
    enum fs_return status;

    src_metadata.parent_index = cwd;
    strcpy(src_metadata.node_name, src);

    fsRead(&src_metadata, &status);
    if (status != FS_SUCCESS) {
        if (status == FS_R_TYPE_IS_DIRECTORY) {
            printString("Cannot copy a directory\n");
        } else if (status == FS_R_NODE_NOT_FOUND) {
            printString("Source file not found\n");
        } else {
            printString("Error reading source file\n");
        }
        return;
    }

    dst_metadata.parent_index = cwd;
    strcpy(dst_metadata.node_name, dst);

    memcpy(dst_metadata.buffer, src_metadata.buffer, src_metadata.filesize);
    dst_metadata.filesize = src_metadata.filesize;

    fsWrite(&dst_metadata, &status);
    if (status != FS_SUCCESS) {
        if (status == FS_W_NODE_ALREADY_EXISTS) {
            printString("Destination file already exists\n");
        } else if (status == FS_W_NO_FREE_NODE) {
            printString("No free nodes available\n");
        } else if (status == FS_W_NO_FREE_DATA) {
            printString("No free data blocks available\n");
        } else if (status == FS_W_NOT_ENOUGH_SPACE) {
            printString("Not enough space to write the file\n");
        } else {
            printString("Error writing destination file\n");
        }
        return;
    }
}

// TODO: 10. Implement cat function
void cat(byte cwd, char* filename) {}
    struct file_metadata metadata;
    enum fs_return status;
    int i;
    char buf[2];

    metadata.parent_index = cwd;
    strcpy(metadata.node_name, filename);

    fsRead(&metadata, &status);

    if (status == FS_SUCCESS) {
        for (i = 0; i < metadata.filesize; ++i) {
            buf[0] = metadata.buffer[i];
            buf[1] = '\0';
            printString(buf);
        }
        printString("\n");
    } else if (status == FS_R_NODE_NOT_FOUND) {
        printString("File not found\n");
    } else if (status == FS_R_TYPE_IS_DIRECTORY) {
        printString("Not a file\n");
    } else {
        printString("Error reading file\n");
    }
}

// TODO: 11. Implement mkdir function
void mkdir(byte cwd, char* dirname) {}
void mkdir(byte cwd, char* dirname) {
    struct node_fs node_fs_buf;
    struct map_fs map_fs_buf;
    int i, new_dir_index;

    readNodeSectors(&node_fs_buf);
    readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == cwd && 
            strcmp(node_fs_buf.nodes[i].node_name, dirname) == 0) {
            printString("Directory already exists\n");
            return;
        }
    }

    new_dir_index = -1;
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].node_name[0] == '\0') {
            new_dir_index = i;
            break;
        }
    }

    if (new_dir_index == -1) {
        printString("No free node available\n");
        return;
    }

    strcpy(node_fs_buf.nodes[new_dir_index].node_name, dirname);
    node_fs_buf.nodes[new_dir_index].parent_index = cwd;
    node_fs_buf.nodes[new_dir_index].data_index = FS_NODE_D_DIR;

    writeSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
    writeSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);

    printString("Directory created successfully\n");
}

