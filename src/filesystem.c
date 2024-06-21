#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

// Function declarations
void readSector(byte* buf, int sector);
void writeSector(byte* buf, int sector);

// Function definitions
void readSector(byte* buf, int sector) {
    int ah = 0x02;                    // read sector service number
    int al = 0x01;                    // number of sectors to read
    int ch = div(sector, 36);         // cylinder number
    int cl = mod(sector, 18) + 1;     // sector number
    int dh = mod(div(sector, 18), 2); // head number
    int dl = 0x00;                    // drive number

    interrupt(
        0x13,
        ah << 8 | al,
        buf,
        ch << 8 | cl,
        dh << 8 | dl
    );
}

void writeSector(byte* buf, int sector) {
    int ah = 0x03;                    // write sector service number
    int al = 0x01;                    // number of sectors to write
    int ch = div(sector, 36);         // cylinder number
    int cl = mod(sector, 18) + 1;     // sector number
    int dh = mod(div(sector, 18), 2); // head number
    int dl = 0x00;                    // drive number

    interrupt(
        0x13,
        ah << 8 | al,
        buf,
        ch << 8 | cl,
        dh << 8 | dl
    );
}

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

  /**
   * add local variable here
   * ...
   */

  readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER);

  /**
   *  add your code here
   * ...
   */
}

// TODO: 3. Implement fsWrite function
void fsWrite(struct file_metadata* metadata, enum fs_return* status) {
  struct map_fs map_fs_buf;
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;
}
