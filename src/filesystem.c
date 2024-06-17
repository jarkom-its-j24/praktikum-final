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
  struct node_item* node = -1;
  int i, j;

  // Read the filesystem nodes and data sectors into memory
  readSector((byte*)&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector((byte*)&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  // Search for the node with the given name and parent index
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].parent_index == metadata->parent_index &&
        strcmp(node_fs_buf.nodes[i].node_name, metadata->node_name) == 0) {
      node = &node_fs_buf.nodes[i];
      break;
    }
  }

  // If node is not found, set status to FS_R_NODE_NOT_FOUND and return
  if (node == -1) {
    *status = FS_R_NODE_NOT_FOUND;
    return;
  }

  // If the found node is a directory, set status to FS_R_TYPE_IS_DIRECTORY and return
  if (node->data_index == FS_NODE_D_DIR) {
    *status = FS_R_TYPE_IS_DIRECTORY;
    return;
  }

  // If the found node is a file, read its data
  metadata->filesize = 0;
  for (i = 0; i < FS_MAX_SECTOR; i++) {
    byte sector = data_fs_buf.datas[node->data_index].sectors[i];
    if (sector == 0x00) break;  // Stop if we reach the end of the sector list

    // Read the sector data into the buffer
    readSector(metadata->buffer + (i * SECTOR_SIZE), sector);
    metadata->filesize += SECTOR_SIZE;
  }

  // Set status to FS_SUCCESS
  *status = FS_SUCCESS;
}


// TODO: 3. Implement fsWrite function
void fsWrite(struct file_metadata* metadata, enum fs_return* status) {
  struct map_fs map_fs_buf;
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;
  int node_index = -1;
  int data_index = -1;
  int free_sectors = 0;
  int i, j;

  // Read the filesystem nodes, data sectors, and map sectors into memory
  readSector((byte*)&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  readSector((byte*)&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  readSector((byte*)&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  // Search for an existing node with the same name and parent index
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].parent_index == metadata->parent_index &&
        strcmp(node_fs_buf.nodes[i].node_name, metadata->node_name) == 0) {
      *status = FS_W_NODE_ALREADY_EXISTS;
      return;
    }
  }

  // Find a free node
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].node_name[0] == '\0') {
      node_index = i;
      break;
    }
  }

  // If no free node is found, set status to FS_W_NO_FREE_NODE and return
  if (node_index == -1) {
    *status = FS_W_NO_FREE_NODE;
    return;
  }

  // Find a free data index
  for (i = 0; i < FS_MAX_DATA; i++) {
    if (data_fs_buf.datas[i].sectors[0] == 0x00) {
      data_index = i;
      break;
    }
  }

  // If no free data index is found, set status to FS_W_NO_FREE_DATA and return
  if (data_index == -1) {
    *status = FS_W_NO_FREE_DATA;
    return;
  }

  // Count free sectors in the map
  for (i = 0; i < SECTOR_SIZE; i++) {
    if (!map_fs_buf.is_used[i]) {
      free_sectors++;
    }
  }

  // If not enough free sectors, set status to FS_W_NOT_ENOUGH_SPACE and return
  if (free_sectors < (metadata->filesize / SECTOR_SIZE + (metadata->filesize % SECTOR_SIZE != 0))) {
    *status = FS_W_NOT_ENOUGH_SPACE;
    return;
  }

  // Set the node properties
  strcpy(node_fs_buf.nodes[node_index].node_name, metadata->node_name);
  node_fs_buf.nodes[node_index].parent_index = metadata->parent_index;
  node_fs_buf.nodes[node_index].data_index = data_index;

  // Write the data to the free sectors
  j = 0;
  for (i = 0; i < SECTOR_SIZE && j < (metadata->filesize / SECTOR_SIZE + (metadata->filesize % SECTOR_SIZE != 0)); i++) {
    if (!map_fs_buf.is_used[i]) {
      data_fs_buf.datas[data_index].sectors[j] = i;
      writeSector(metadata->buffer + j * SECTOR_SIZE, i);
      map_fs_buf.is_used[i] = true;
      j++;
    }
  }

  // Write back the updated filesystem structures to the disk
  writeSector((byte*)&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  writeSector((byte*)&node_fs_buf, FS_NODE_SECTOR_NUMBER);
  writeSector((byte*)&data_fs_buf, FS_DATA_SECTOR_NUMBER);

  // Set status to FS_SUCCESS
  *status = FS_SUCCESS;
}
