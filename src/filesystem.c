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
    int i, j;

    // Membaca sektor node dan data dari filesystem
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
    readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

    // Iterasi setiap item node untuk mencari node yang sesuai
    for (i = 0; i < FS_NODE_COUNT; i++) {
        if (node_fs_buf.nodes[i].parent_node_index == metadata->parent_index && 
            strcmp(node_fs_buf.nodes[i].name, metadata->node_name) == 0) {
            // Node ditemukan
            if (node_fs_buf.nodes[i].sector_entry_index == FS_NODE_S_IDX_FOLDER) {
                // Jika node adalah direktori
                *status = FS_R_TYPE_IS_DIRECTORY;
                return;
            } else {
                // Jika node adalah file
                metadata->filesize = 0;
                // Iterasi setiap sektor data dari node yang ditemukan
                for (j = 0; j < FS_MAX_SECTOR; j++) {
                    byte sector = node_fs_buf.nodes[i].sector_entry_index[j];
                    if (sector == 0x00) {
                        break; // Jika sektor tidak valid, hentikan iterasi
                    }
                    readSector(metadata->buffer + (j * SECTOR_SIZE), sector);
                    metadata->filesize += SECTOR_SIZE;
                }
                *status = FS_R_SUCCESS;
                return;
            }
        }
    }
    // Node tidak ditemukan
    *status = FS_R_NODE_NOT_FOUND;
}

// TODO: 3. Implement fsWrite function
void fsWrite(struct file_metadata* metadata, enum fs_return* status) {
  struct map_fs map_fs_buf;
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;
  int i, j, free_node_index = -1, free_data_index = -1;
    int free_blocks = 0;
    
    // Membaca filesystem dari disk ke memory
    readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
    readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
    
    // Mencari node yang memiliki nama yang sama dengan metadata->node_name dan parent index yang sama
    for (i = 0; i < FS_NODE_COUNT; i++) {
        if (node_fs_buf.nodes[i].parent_node_index == metadata->parent_index && 
            strcmp(node_fs_buf.nodes[i].name, metadata->node_name) == 0) {
            // Node yang dicari ditemukan
            *status = FS_R_NODE_ALREADY_EXISTS;
            return;
        }
        // Cari node yang kosong
        if (free_node_index == -1 && node_fs_buf.nodes[i].name[0] == '\0') {
            free_node_index = i;
        }
    }
    if (free_node_index == -1) {
        // Tidak ada node yang kosong
        *status = FS_W_NO_FREE_NODE;
        return;
    }
    
    // Mencari data yang kosong
    for (i = 0; i < FS_MAX_SECTOR; i++) {
        if (data_fs_buf.sector[i].sector[0] == 0x00) {
            free_data_index = i;
            break;
        }
    }
    if (free_data_index == -1) {
        // Tidak ada data yang kosong
        *status = FS_W_NO_FREE_DATA;
        return;
    }
    
    // Menghitung blok yang kosong di map
    for (i = 0; i < FS_MAP_SECTOR_SIZE; i++) {
        if (!map_fs_buf.is_used[i]) {
            free_blocks++;
        }
    }
    if (free_blocks < metadata->filesize / SECTOR_SIZE) {
        // Tidak cukup blok yang kosong
        *status = FS_W_NOT_ENOUGH_SPACE;
        return;
    }
    
    // Set nama node, parent index, dan data index
    strcpy(node_fs_buf.nodes[free_node_index].name, metadata->node_name);
    node_fs_buf.nodes[free_node_index].parent_node_index = metadata->parent_index;
    node_fs_buf.nodes[free_node_index].sector_entry_index[0] = free_data_index;
    
    if (metadata->filesize == 0) {
        // Jika filesize adalah 0, maka file yang akan ditulis adalah direktori
        node_fs_buf.nodes[free_node_index].sector_entry_index[0] = FS_NODE_S_IDX_FOLDER;
        *status = FS_W_SUCCESS;
        return;
    }
    
    // Penulisan data ke sektor-sektor yang kosong
    j = 0;
    for (i = 0; i < FS_MAP_SECTOR_SIZE && j < metadata->filesize / SECTOR_SIZE; i++) {
        if (!map_fs_buf.is_used[i]) {
            // Tulis data ke sektor yang kosong
            map_fs_buf.is_used[i] = true;
            data_fs_buf.sector[free_data_index].sector[j] = i;
            writeSector(metadata->buffer + j * SECTOR_SIZE, i);
            j++;
        }
    }
    
    // Tulis kembali filesystem yang telah diubah ke dalam disk
    writeSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
    writeSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    writeSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);
    writeSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
    
    *status = FS_W_SUCCESS;
}
}
