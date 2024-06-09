#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "std_type.h"

#define MAX_FILENAME 14

#define FS_MAP_SECTOR_NUMBER 0x100
#define FS_NODE_SECTOR_NUMBER 0x101
#define FS_DATA_SECTOR_NUMBER 0x103

#define FS_NODE_P_ROOT 0xFF
#define FS_NODE_D_DIR 0xFF

#define FS_MAX_NODE 64
#define FS_MAX_DATA 32
#define FS_MAX_SECTOR 16

struct map_fs {
  bool is_used[SECTOR_SIZE];
};

struct node_item {
  byte parent_index;
  byte data_index;
  char node_name[MAX_FILENAME];
};

struct node_fs {
  struct node_item nodes[FS_MAX_NODE];
};

struct data_item {
  byte sectors[FS_MAX_SECTOR];
};

struct data_fs {
  struct data_item datas[FS_MAX_DATA];
};

struct file_metadata {
  byte parent_index;
  unsigned int filesize;
  char node_name[MAX_FILENAME];
  byte buffer[FS_MAX_SECTOR * SECTOR_SIZE];
};

enum fs_return {
  FS_UNKOWN_ERROR = -1,
  FS_SUCCESS = 0,

  FS_R_NODE_NOT_FOUND = 1,
  FS_R_TYPE_IS_DIRECTORY = 2,

  FS_W_NODE_ALREADY_EXISTS = 3,
  FS_W_NOT_ENOUGH_SPACE = 4,
  FS_W_NO_FREE_NODE = 5,
  FS_W_NO_FREE_DATA = 6,
  FS_W_INVALID_DIRECTORY = 7,
};

void fsInit();
void fsRead(struct file_metadata* metadata, enum fs_return* status);
void fsWrite(struct file_metadata* metadata, enum fs_return* status);

#endif // __FILESYSTEM_H__