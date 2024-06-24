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

// Implement printCWD function
void printCWD(byte cwd) {
  struct node_fs node_fs_buf;
  char path[256];
  int i;
  int length = 0;

  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);

  if (cwd == FS_NODE_P_ROOT) {
    printString("/");
    return;
  }

  while (cwd != FS_NODE_P_ROOT) {
    for (i = strlen(node_fs_buf.nodes[cwd].node_name) - 1; i >= 0; i--) {
      path[length++] = node_fs_buf.nodes[cwd].node_name[i];
    }
    path[length++] = '/';
    cwd = node_fs_buf.nodes[cwd].parent_index;
  }
  path[length] = '\0';

  for (i = length - 1; i >= 0; i--) {
    interrupt(0x10, 0xE << 8 | path[i], 0, 0, 0);
  }
}

// Implement parseCommand function
void parseCommand(char* buf, char* cmd, char arg[2][64]) {
  int i = 0;
  int j = 0;
  int arg_idx = 0;

  // Skip leading whitespace
  while (buf[i] == ' ') i++;

  // Read command
  while (buf[i] != ' ' && buf[i] != '\0') {
    cmd[j++] = buf[i++];
  }
  cmd[j] = '\0';

  // Skip whitespace before arguments
  while (buf[i] == ' ') i++;

  // Read arguments
  while (buf[i] != '\0') {
    j = 0;
    while (buf[i] != ' ' && buf[i] != '\0') {
      arg[arg_idx][j++] = buf[i++];
    }
    arg[arg_idx][j] = '\0';
    arg_idx++;
    while (buf[i] == ' ') i++;
  }

  // Fill remaining argument slots with empty strings
  for (; arg_idx < 2; arg_idx++) {
    arg[arg_idx][0] = '\0';
  }
}

void cd(byte* cwd, char* dirname) {
    struct node_fs node_fs_buf;
    int i;

    // Read the node sectors into memory
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);

    // Handle special case: cd ..
    if (strcmp(dirname, "..") == 1) {
        if (*cwd != FS_NODE_P_ROOT) {
            *cwd = node_fs_buf.nodes[*cwd].parent_index;
        }
        return;
    }

    // Handle special case: cd /
    if (strcmp(dirname, "/") == 1) {
        *cwd = FS_NODE_P_ROOT;
        return;
    }

    // Iterate through the nodes to find the target directory by name
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == *cwd &&
            strcmp(node_fs_buf.nodes[i].node_name, dirname) == 1) {
            // Check if the found node is a directory
            if (node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR) {
                *cwd = i;
            } else {
                printString("Not a directory!\n");
            }
            return;
        }
    }

    // If target directory is not found, print an error message
    printString("Directory not found\n");
}

void ls(byte cwd, char* dirname) {
    struct node_fs node_fs_buf;
    int i;
    byte target_cwd = cwd;

    // Baca sektor node dari file system
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);

    if (strcmp(dirname, ".") != 0 && strlen(dirname) > 0) {
        // Cari direktori yang sesuai dengan dirname
        for (i = 0; i < FS_MAX_NODE; i++) {
            if (node_fs_buf.nodes[i].parent_index == cwd &&
                strcmp(node_fs_buf.nodes[i].node_name, dirname) == 0 &&
                node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR) {
                target_cwd = i;
                break;
            }
        }
        if (i == FS_MAX_NODE) {
            printString("Directory not found\n");
            return;
        }
    }

    // Tampilkan isi direktori
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == target_cwd) {
            printString(node_fs_buf.nodes[i].node_name);
            if (node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR) {
                printString("/");
            }
            printString(" ");
        }
    }
    printString("\n");
}


void mv(byte cwd, char* src, char* dst) {
    struct node_fs node_fs_buf;
    int i, src_index = -1, dst_dir_index = cwd;
    char* outputname;
    
    // Baca sektor node dari file system
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);

    // Cari sumber file
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == cwd && strcmp(node_fs_buf.nodes[i].node_name, src) == 0) {
            src_index = i;
            break;
        }
    }
    
    if (src_index == -1) {
        printString("File not found\n");
        return;
    }

    // Parsing dst untuk mendapatkan direktori tujuan dan nama output
    if (dst[0] == '/') {
        dst_dir_index = FS_NODE_P_ROOT;
        outputname = dst + 1;
    } else if (dst[0] == '.' && dst[1] == '.' && dst[2] == '/') {
        dst_dir_index = node_fs_buf.nodes[cwd].parent_index;
        outputname = dst + 3;
    } else {
        outputname = dst;
    }

    // Cek jika dst adalah direktori
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == dst_dir_index && strcmp(node_fs_buf.nodes[i].node_name, outputname) == 0) {
            printString("Destination already exists\n");
            return;
        }
    }

    // Pindahkan file
    node_fs_buf.nodes[src_index].parent_index = dst_dir_index;
    strcpy(node_fs_buf.nodes[src_index].node_name, outputname);

    // Tulis kembali sektor node ke file system
    writeSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    writeSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);

    printString("File moved\n");
}

void cp(byte cwd, char* src, char* dst) {
    struct node_fs node_fs_buf;
    struct data_fs data_fs_buf;
    struct file_metadata file_meta;
    enum fs_return status;
    int i, src_index = -1, dst_dir_index = cwd;
    char* outputname;

    // Baca sektor node dan data dari file system
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);
    readSector(&(data_fs_buf.datas[0]), FS_DATA_SECTOR_NUMBER);
    readSector(&(data_fs_buf.datas[16]), FS_DATA_SECTOR_NUMBER);

    // Cari sumber file
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == cwd && strcmp(node_fs_buf.nodes[i].node_name, src) == 0) {
            src_index = i;
            break;
        }
    }

    if (src_index == -1) {
        printString("File not found\n");
        return;
    }

    // Parsing dst untuk mendapatkan direktori tujuan dan nama output
    if (dst[0] == '/') {
        dst_dir_index = FS_NODE_P_ROOT;
        outputname = dst + 1;
    } else if (dst[0] == '.' && dst[1] == '.' && dst[2] == '/') {
        dst_dir_index = node_fs_buf.nodes[cwd].parent_index;
        outputname = dst + 3;
    } else {
        outputname = dst;
    }

    // Cek jika dst adalah direktori
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == dst_dir_index && strcmp(node_fs_buf.nodes[i].node_name, outputname) == 0) {
            printString("Destination already exists\n");
            return;
        }
    }

    // Set metadata file untuk penyalinan
    file_meta.parent_index = dst_dir_index;
    file_meta.filesize = 0;
    strcpy(file_meta.node_name, outputname);
    
    for (i = 0; i < FS_MAX_SECTOR; i++) {
        file_meta.buffer[i] = 0;
    }

    fsRead(&file_meta, &status);
    if (status != FS_SUCCESS) {
        printString("Error reading file\n");
        return;
    }

    fsWrite(&file_meta, &status);
    if (status != FS_SUCCESS) {
        printString("Error writing file\n");
        return;
    }

    printString("File copied\n");
}

void cat(byte cwd, char* filename) {
    struct node_fs node_fs_buf;
    struct file_metadata file_meta;
    enum fs_return status;
    int i, file_index = -1;

    // Baca sektor node dari file system
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);

    // Cari file
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == cwd && strcmp(node_fs_buf.nodes[i].node_name, filename) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        printString("File not found\n");
        return;
    }

    // Baca isi file
    file_meta.parent_index = cwd;
    strcpy(file_meta.node_name, filename);

    fsRead(&file_meta, &status);
    if (status != FS_SUCCESS) {
        printString("Error reading file\n");
        return;
    }

    // Tampilkan isi file
    for (i = 0; i < file_meta.filesize; i++) {
        interrupt(0x10, 0xE << 8 | file_meta.buffer[i], 0, 0, 0);
    }
    printString("\n");
}


void mkdir(byte cwd, char* dirname) {
    struct node_fs node_fs_buf;
    struct map_fs map_fs_buf;
    int i, empty_node_index = -1;

    // Baca sektor node dan map dari file system
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
    readSector(&(map_fs_buf.is_used[0]), FS_MAP_SECTOR_NUMBER);

    // Cek apakah direktori sudah ada
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == cwd && strcmp(node_fs_buf.nodes[i].node_name, dirname) == 0) {
            printString("Directory already exists\n");
            return;
        }
    }

    // Cari node yang kosong
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (!map_fs_buf.is_used[i]) {
            empty_node_index = i;
            break;
        }
    }

    if (empty_node_index == -1) {
        printString("No free nodes available\n");
        return;
    }

    // Buat direktori baru
    strcpy(node_fs_buf.nodes[empty_node_index].node_name, dirname);
    node_fs_buf.nodes[empty_node_index].parent_index = cwd;
    node_fs_buf.nodes[empty_node_index].data_index = FS_NODE_D_DIR;
    map_fs_buf.is_used[empty_node_index] = true;

    // Tulis kembali sektor node dan map ke file system
    writeSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    writeSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
    writeSector(&(map_fs_buf.is_used[0]), FS_MAP_SECTOR_NUMBER);

    printString("Directory created\n");
}





