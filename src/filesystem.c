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
    int i, j, data_index;
    int buffer_index = 0;

    readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
    readSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
    readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == metadata->parent_index &&
            strcmp(node_fs_buf.nodes[i].node_name, metadata->node_name) == 0) {

            if (node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR) {
                *status = FS_R_TYPE_IS_DIRECTORY;
                return;
            }

            data_index = node_fs_buf.nodes[i].data_index;
            metadata->filesize = 0;

            for (j = 0; j < FS_MAX_SECTOR; j++) {
                if (data_fs_buf.datas[data_index].sectors[j] == 0x00) {
                    break;
                }

                readSector(metadata->buffer + buffer_index, data_fs_buf.datas[data_index].sectors[j]);
                buffer_index += SECTOR_SIZE;
                metadata->filesize += SECTOR_SIZE;

                if (buffer_index >= (FS_MAX_SECTOR * SECTOR_SIZE)) {
                    metadata->filesize = (FS_MAX_SECTOR * SECTOR_SIZE);
                    break;
                }
            }

            *status = FS_SUCCESS;
            return;
        }
    }

    *status = FS_R_NODE_NOT_FOUND;
}

// TODO: 3. Implement fsWrite function
void fsWrite(struct file_metadata* metadata, enum fs_return* status) {
    struct map_fs map_fs_buf;
    struct node_fs node_fs_buf;
    struct data_fs data_fs_buf;
    int i, j, empty_node_index = -1, empty_data_index = -1;
    int required_blocks, sector_indices[FS_MAX_SECTOR], empty_blocks_count = 0;

    readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
    readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
    readSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
    readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (strcmp(node_fs_buf.nodes[i].node_name, metadata->node_name) == 0 &&
            node_fs_buf.nodes[i].parent_index == metadata->parent_index) {
            *status = FS_W_NODE_ALREADY_EXISTS;
            return;
        }
    }

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].node_name[0] == '\0') {
            empty_node_index = i;
            break;
        }
    }

    if (empty_node_index == -1) {
        *status = FS_W_NO_FREE_NODE;
        return;
    }

    required_blocks = (metadata->filesize + SECTOR_SIZE - 1) / SECTOR_SIZE;

    for (i = 0, j = 0; i < SECTOR_SIZE && j < required_blocks; i++) {
        if (!map_fs_buf.is_used[i]) {
            sector_indices[j++] = i;
            empty_blocks_count++;
        }
    }

    if (empty_blocks_count < required_blocks) {
        *status = FS_W_NOT_ENOUGH_SPACE;
        return;
    }

    for (i = 0; i < FS_MAX_DATA; i++) {
        if (data_fs_buf.datas[i].sectors[0] == 0x00) {
            empty_data_index = i;
            break;
        }
    }

    if (empty_data_index == -1) {
        *status = FS_W_NO_FREE_DATA;
        return;
    }

    strcpy(node_fs_buf.nodes[empty_node_index].node_name, metadata->node_name);
    node_fs_buf.nodes[empty_node_index].parent_index = metadata->parent_index;
    node_fs_buf.nodes[empty_node_index].data_index = empty_data_index;

    for (i = 0; i < required_blocks; i++) {
        writeSector(metadata->buffer + i * SECTOR_SIZE, sector_indices[i]);
        map_fs_buf.is_used[sector_indices[i]] = true;
        data_fs_buf.datas[empty_data_index].sectors[i] = sector_indices[i];
    }

    for (; i < FS_MAX_SECTOR; i++) {
        data_fs_buf.datas[empty_data_index].sectors[i] = 0x00;
    }

    writeSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
    writeSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
    writeSector(((byte*)&node_fs_buf) + SECTOR_SIZE, FS_NODE_SECTOR_NUMBER + 1);
    writeSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

    *status = FS_SUCCESS;
}
