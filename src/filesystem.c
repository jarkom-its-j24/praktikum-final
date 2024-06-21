#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

void fsInit() {
  struct map_fs map_fs_buf;
  int i = 0;

  readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  for (i = 0; i < 16; i++) map_fs_buf.is_used[i] = true;
  for (i = 256; i < 512; i++) map_fs_buf.is_used[i] = true;
  writeSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
}

// TODO: 2. Implement fsRead function
void fsRead(struct file_metadata* metadata, enum fs_return* status) {
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;

  int i,j;
  int found;
  struct node_item *node;
  found = -1;

  readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);

  for (i=0; i<FS_MAX_NODE; i++) {
    if (strcmp(node_fs_buf.nodes[i].node_name, metadata->node_name) && 
    node_fs_buf.nodes[i].parent_index == metadata->parent_index) {
      node = &node_fs_buf.nodes[i];
      found = 1; break;
    }
  }

  if (found == -1) {
    *status = FS_R_NODE_NOT_FOUND;
    return;
  }

  if (node->data_index == FS_NODE_D_DIR) {
    *status = FS_R_TYPE_IS_DIRECTORY;
    return;
  }

  metadata->filesize = 0;

  for (i=0; i<FS_MAX_SECTOR; i++) {
    if (data_fs_buf.datas[node->data_index].sectors[i] == 0x00) {
      break;
    }
    readSector(metadata->buffer + i * SECTOR_SIZE, data_fs_buf.datas[node->data_index].sectors[i]);
    metadata->filesize += SECTOR_SIZE;
  }

  *status = FS_SUCCESS;
}

// TODO: 3. Implement fsWrite function
void fsWrite(struct file_metadata* metadata, enum fs_return* status) {
  struct map_fs map_fs_buf;
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;

  int i,j;
  int free_node_idx = -1;
  int free_data_idx = -1;
  int free_blocks = 0;

  readSector((byte*)&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);
  readSector((byte*)&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  for (i=0; i<FS_MAX_NODE; i++) {
    if (strcmp(node_fs_buf.nodes[i].node_name, metadata->node_name) && 
    node_fs_buf.nodes[i].parent_index == metadata->parent_index) {
      *status = FS_W_NODE_ALREADY_EXISTS;
      return;
    }
  }

  for (i=0; i<FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].node_name[0] == '\0') {
      free_node_idx = i; break;
    }
  }
  
  if (free_node_idx == -1) {
    *status = FS_W_NO_FREE_NODE;
    return;
  }

  if (metadata->filesize > 0) {
    for (i=0; i<FS_MAX_DATA; i++) {
      if (data_fs_buf.datas[i].sectors[0] == 0x00) {
        free_data_idx = i; break;
      }
    }

    if (free_data_idx == -1) {
      *status = FS_W_NO_FREE_DATA;
      return;
    }

    for (i = 0; i < SECTOR_SIZE; i++) {
        if (!map_fs_buf.is_used[i]) {
            free_blocks++;
        } 
    }

    if (free_blocks < (metadata->filesize + SECTOR_SIZE - 1) / SECTOR_SIZE) {
      *status = FS_W_NOT_ENOUGH_SPACE;
      return;
    }

  } else {
    free_data_idx = FS_NODE_D_DIR;
  }

  strcpy(node_fs_buf.nodes[free_node_idx].node_name, metadata->node_name);
  node_fs_buf.nodes[free_node_idx].parent_index = metadata->parent_index;
  node_fs_buf.nodes[free_node_idx].data_index = free_data_idx;

  if (metadata->filesize > 0) {
      for (i = 0; i < SECTOR_SIZE && j < (metadata->filesize + SECTOR_SIZE - 1) / SECTOR_SIZE; i++) {
          if (!map_fs_buf.is_used[i]) {
              data_fs_buf.datas[free_data_idx].sectors[j] = i;
              writeSector(metadata->buffer + j * SECTOR_SIZE, i);
              map_fs_buf.is_used[i] = true;
              j++;
          }
      }
  }

  writeSector((byte*)&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  writeSector((byte*)&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  writeSector((byte*)&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  *status = FS_SUCCESS;
}
