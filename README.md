# MengOS

Apa??? Bikin kernel lagi???? :(

[Looking for the English version?](./README-EN.md)

## Pengenalan

Pada final praktikum, kita akan melanjutkan `task-4` dari praktikum modul 4 yang sebelumnya. Kali ini, kita akan membuat sebuah filesystem sederhana yang dapat digunakan untuk menyimpan file-file yang kita buat. Filesystem yang akan kita buat ini akan menggunakan metode penyimpanan data yang sederhana, yaitu dengan menyimpan data file ke dalam blok-blok yang telah disediakan oleh filesystem. Jika kalian sudah tidak sabar ingin langsung mengerjakan task-task yang ada, bisa search `TODO` pada workspace ini. Berikut adalah gambaran yang akan kalian kerjakan pada final praktikum kali ini.

- Membuat filesystem yang dapat digunakan untuk menyimpan file-file yang kita buat.
- Melengkapi kernel untuk dapat membaca dan menulis file ke dalam filesystem yang telah kita buat.
- Membuat shell sederhana yang dapat digunakan untuk mengakses filesystem yang telah kita buat.

## Pencerdasan

Penjelasan pada praktikum final akan sering menggunakan angka heksadesimal. Penggunaan angka heksadesimal ditandai dengan prefix `0x`. Jika kalian belum terbiasa dengan angka heksadesimal, kalian dapat menggunakan kalkulator yang mendukung mode heksadesimal atau menggunakan konversi angka heksadesimal ke desimal.

### Struktur Disk

Jika kalian sudah melewati modul 4, pasti sudah tidak asing lagi dengan struktur disk yang akan kita gunakan. Disk yang kita gunakan terdiri dari beberapa blok. Selanjutnya blok akan disebut sektor. Setiap sektor memiliki ukuran 512 bytes. Sektor pertama akan digunakan sebagai boot sector, yang berisi hasil kompilasi dari `bootloader.asm`. Sektor kedua hingga sektor ke-15 akan digunakan untuk menyimpan kode teks dari kernel yang kita buat.

Dengan melihat hasil dari modul 4, berikut adalah struktur disk yang akan kita gunakan. Dapat dilihat menggunakan aplikasi seperti `HxD` atau menggunakan perintah `hexdump` atau `xxd`.

![struktur-disk-sektor](./assets/struktur-disk-sektor.png)

Untuk memudahkan dalam ilustrasi selanjutnya, struktur disk akan digambarkan sebagai berikut.

![struktur-disk-legend](./assets/struktur-disk-legend.png)

Satu sektor akan digambarkan sebagai satu blok. Alamat sektor akan dinomori ulang dari `0x00`. Sehingga sektor pertama akan memiliki alamat `0x00`, sektor kedua akan memiliki alamat `0x01`, dan seterusnya. Satu baris akan berisi 16 sektor. Sehingga baris pertama akan berisi sektor dengan alamat `0x00` hingga `0x0F`, baris kedua akan berisi sektor dengan alamat `0x10` hingga `0x1F`, dan seterusnya.

Untuk mencari alamat sektor pada isi file `floppy.img`, kita dapat mengonversi alamat sektor ke dalam alamat byte dengan cara seperti pada gambar di atas.

### Struktur Filesystem

Filesystem yang akan dibuat akan menggunakan beberapa komponen, yaitu map, node, dan data. Map akan disimpan sebanyak 1 sektor pada sektor `0x100`. Node akan disimpan sebanyak 2 sektor pada sektor `0x101` dan `0x102`. Data akan disimpan sebanyak 1 sektor pada sektor `0x103`.

Berikut adalah ilustrasi dari struktur filesystem yang akan kita buat.

![struktur-filesystem](./assets/struktur-filesystem.png)

### Struktur Filesystem Map

Map akan digunakan untuk menandai blok-blok pada disk yang telah digunakan oleh file. Setiap blok akan memiliki status `0x00` jika sektor yang bersangkutan belum digunakan, dan `0x01` jika sektor yang bersangkutan telah digunakan. Contohnya, karena pada sektor `0x00` telah digunakan oleh bootloader, maka isi dari map ke-0 adalah `0x01`. Komponen map akan digunakan ketika kita ingin menulis file ke dalam disk untuk mengetahui sektor mana saja yang dapat kita gunakan.

Berikut adalah ilustrasi dari komponen map.

![struktur-map](./assets/struktur-map.png)

Map akan berukuran 1 sektor (512 bytes). Item ke-0 hingga item ke-15 pada map akan memiliki status `0x01` karena telah digunakan oleh sistem operasi. Item ke-16 hingga item ke-255 akan memiliki status `0x00` karena belum digunakan. Mulai dari item ke-256 (`0x100`) hingga item ke 511 (`0x1FF`) akan ditandai sebagai sektor yang telah digunakan. Hal ini dikarenakan kita tidak memperbolehkan file untuk menulis data pada sektor yang berada di atas sektor `0x100`.

### Struktur Filesystem Node

Node akan digunakan untuk menyimpan informasi dari file atau direktori yang kita buat. Setiap node akan memiliki ukuran 16 bytes. Sehingga, total akan terdapat 64 item node yang bisa disimpan. Berikut adalah ilustrasi dari komponen node.

![struktur-node](./assets/struktur-node.png)

Berikut adalah penjelasan dari setiap item pada node.

- **P**: kolom pertama pada item node berguna sebagai penunjuk _parent node_ dari node yang bersangkutan dan akan bernilai `0xFF` jika parent dari node yang bersangkutan adalah _root node_.

  Sebagai contoh, pada node index ke-1, nilai dari kolom pertama adalah `0x00`. Hal ini menunjukkan bahwa node index ke-1 adalah _parent node_ dari node index ke-0. Sedangkan pada node index ke-0, nilai dari kolom pertama adalah `0xFF`. Hal ini menunjukkan bahwa _parent node_ dari node index ke-0 adalah _root node_.

- **D**: kolom kedua pada item node berguna sebagai penunjuk index dari komponen data yang akan digunakan untuk menyimpan data file. Jika nilai dari kolom kedua adalah `0xFF`, maka node tersebut merupakan direktori.

  Sebagai contoh, pada node index ke-0, nilai dari kolom kedua adalah `0x00`. Hal tersebut berarti informasi data dari file tersebut bisa diakses pada komponen data index ke-0. Sedangkan pada node index ke-1, nilai dari kolom kedua adalah `0xFF`. Hal tersebut berarti node tersebut merupakan direktori.

- **Node name**: kolom ketiga hingga kolom terakhir pada item node berguna sebagai nama dari node tersebut. Nama dari node akan memiliki panjang maksimal 13 karakter (karakter terakhir adalah karakter null).

### Struktur Filesystem Data

Komponen data akan digunakan untuk petunjuk sektor-sektor yang digunakan untuk menyimpan data file. Setiap item data akan memiliki ukuran 16 bytes. Sehingga, total akan terdapat 32 item data yang bisa disimpan. Berikut adalah ilustrasi dari komponen data.

![struktur-data](./assets/struktur-data.png)

Setiap kolom pada item data akan menunjukkan alamat sektor yang digunakan untuk menyimpan data file. Karena satu byte hanya dapat menunjukkan alamat sektor hingga 255 (`0xFF`), maka kita hanya dapat menyimpan alamat sektor hingga sektor `0xFF`. Oleh karena itu, item map ke-256 hingga akhir akan ditandai sebagai sektor yang telah digunakan.

### Ilustrasi Filesystem

Berikut adalah ilustrasi dari ketiga komponen filesystem yang telah dijelaskan sebelumnya.

![filesystem-illustration](./assets/filesystem-illustration.png)

## Instruksi Pengerjaan

### Task 1 - Membuat syscall readSector dan writeSector

Pada task ini, kalian diminta untuk membuat syscall `readSector` dan `writeSector` yang akan digunakan untuk membaca dari disk ke memory dan menulis dari memory ke disk.

Berikut adalah implementasi dari `readSector` dan penjelasannya.

```c
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
```

- Interrupt vector yang akan digunakan adalah `0x13` untuk melakukan operasi disk I/O.

- Register `ah` akan diisi dengan `0x02` yang menunjukkan operasi `read`.

- Register `al` akan diisi dengan `0x01` yang menunjukkan jumlah sektor yang akan dibaca.

- Register `ch` dan `cl` akan diisi dengan nomor cylinder dan sector yang akan dibaca.

  Pada floppy disk, terdapat 2 head, 18 sector per track, dan 36 track per cylinder. Sehingga, nomor cylinder akan dihitung dengan membagi nomor sektor dengan 36. Sedangkan nomor sector akan dihitung dengan mengambil sisa pembagian nomor sektor dengan 18 dan ditambahkan dengan 1.

- Register `dh` dan `dl` akan diisi dengan nomor head dan drive yang akan digunakan.

  Pada floppy disk, terdapat 2 head. Sehingga, nomor head akan dihitung dengan membagi nomor sektor dengan 18 dan mengambil sisa pembagian dengan 2. Sedangkan nomor drive akan diisi dengan `0x00` yang menunjukkan drive pertama.

Untuk `writeSector`, kalian dapat menggunakan implementasi yang sama dengan `readSector` dengan mengganti nilai register `ah` dengan `0x03` yang menunjukkan operasi `write`.

### Task 2 - Implementasi fsRead

Pada [`filesystem.h`](./src/filesystem.h), terdapat beberapa konstanta dan tipe data yang akan digunakan untuk membantu dalam implementasi filesystem. Kalian diminta untuk mengimplementasikan fungsi `fsRead` yang akan digunakan untuk membaca direktori atau file dari filesystem. Fungsi `fsRead` akan menerima parameter sebagai berikut.

```c
void fsRead(struct file_metadata* metadata, enum fs_return* status);
```

- `metadata` adalah pointer ke `file_metadata` yang akan digunakan untuk menyimpan informasi dari file atau direktori yang akan dibaca.

  Struktur `file_metadata` akan memiliki struktur sebagai berikut.

  ```c
  struct file_metadata {
    byte parent_index;
    unsigned int filesize;
    char node_name[MAX_FILENAME];
    byte buffer[FS_MAX_SECTOR * SECTOR_SIZE];
  };
  ```

  - `parent_index` adalah index dari _parent node_ dari file atau direktori yang akan dibaca.
  - `filesize` adalah ukuran dari file yang akan dibaca. `filesize` berisi 0 dalam pemanggilan fungsi `fsRead`.
  - `node_name` adalah nama dari file atau direktori yang akan dibaca.
  - `buffer` adalah pointer ke buffer yang akan digunakan untuk menyimpan data dari file atau direktori yang akan dibaca. `buffer` berisi `0x00` dalam pemanggilan fungsi `fsRead`.

- `status` adalah pointer ke `fs_return` yang akan digunakan untuk menyimpan status dari operasi yang dilakukan.

Langkah-langkah yang harus dilakukan pada fungsi `fsRead` adalah sebagai berikut.

1. Membaca filesystem dari disk ke memory.

2. Iterasi setiap item node untuk mencari node yang memiliki nama yang sesuai dengan `metadata->node_name` dan parent index sesuai dengan `metadata->parent_index`.

3. Jika node yang dicari tidak ditemukan, maka set `status` dengan `FS_R_NODE_NOT_FOUND`.

4. Jika node yang ditemukan adalah direktori, maka set `status` dengan `FS_R_TYPE_IS_DIRECTORY`.

5. Jika node yang ditemukan adalah file, maka proses selanjutnya adalah sebagai berikut.

   - Set `metadata->filesize` dengan 0.
   - Lakukan iterasi i dari 0 hingga `FS_MAX_SECTOR`
   - Jika data index ke-i dari node yang ditemukan adalah `0x00`, maka hentikan iterasi.
   - Lakukan `readSector` untuk membaca data dari sektor yang ditunjuk oleh data pada _data index_ dengan sectors ke-i disimpan ke dalam `metadata->buffer + i * SECTOR_SIZE`.
   - Tambahkan `SECTOR_SIZE` ke `metadata->filesize`.

6. Set `status` dengan `FS_R_SUCCESS`.

### Task 3 - Implementasi fsWrite

Selanjutnya kalian diminta untuk mengimplementasikan fungsi `fsWrite` yang akan digunakan untuk menulis file ke dalam filesystem. Fungsi `fsWrite` akan menerima parameter yang sama dengan `fsRead` sebagai berikut.

```c
void fsWrite(struct file_metadata* metadata, enum fs_return* status);
```

Pada fungsi `fsWrite`, `metadata` yang diterima akan berisi informasi berikut.

- `parent_index` adalah index dari _parent node_ dari file yang akan ditulis. Jika `parent_index` adalah `0xFF`, maka file yang akan ditulis akan disimpan pada _root directory_.
- `filesize` adalah ukuran dari file yang akan ditulis. Jika `filesize` adalah 0, maka file yang akan ditulis adalah direktori.
- `node_name` adalah nama dari file yang akan ditulis.
- `buffer` adalah pointer ke buffer yang berisi data dari file yang akan ditulis.

Langkah-langkah yang harus dilakukan pada fungsi `fsWrite` adalah sebagai berikut.

1. Membaca filesystem dari disk ke memory.

2. Lakukan iterasi setiap item node untuk mencari node yang memiliki nama yang sama dengan `metadata->node_name` dan parent index yang sama dengan `metadata->parent_index`. Jika node yang dicari ditemukan, maka set `status` dengan `FS_R_NODE_ALREADY_EXISTS` dan keluar.

3. Selanjutnya, cari node yang kosong (nama node adalah string kosong) dan simpan index-nya. Jika node yang kosong tidak ditemukan, maka set `status` dengan `FS_W_NO_FREE_NODE` dan keluar.

4. Iterasi setiap item data untuk mencari data yang kosong (alamat sektor data ke-0 adalah `0x00`) dan simpan index-nya. Jika data yang kosong tidak ditemukan, maka set `status` dengan `FS_W_NO_FREE_DATA` dan keluar.

5. Iterasi setiap item map dan hitung blok yang kosong (status blok adalah `0x00` atau `false`). Jika blok yang kosong kurang dari `metadata->filesize / SECTOR_SIZE`, maka set `status` dengan `FS_W_NOT_ENOUGH_SPACE` dan keluar.

6. Set nama dari node yang ditemukan dengan `metadata->node_name`, parent index dengan `metadata->parent_index`, dan data index dengan index data yang kosong.

7. Lakukan penulisan data dengan cara sebagai berikut.

   - Buatlah variabel counter yang akan digunakan untuk menghitung jumlah sektor yang telah ditulis (akan disebut dengan j).

   - Lakukan iterasi i dari 0 hingga `SECTOR_SIZE`.

   - Jika item map pada index ke-i adalah `0x00`, maka tulis index i ke dalam data item sektor ke-j dan tulis data dari buffer ke dalam sektor ke-i.

   - Penulisan dapat menggunakan fungsi `writeSector` dari `metadata->buffer + i * SECTOR_SIZE`.

   - Tambahkan 1 ke j.

8. Tulis kembali filesystem yang telah diubah ke dalam disk.

9. Set `status` dengan `FS_W_SUCCESS`.

### Task 4 - Implementasi printCWD

Setelah berhasil mengimplementasikan fungsi `fsRead` dan `fsWrite`, selanjutnya adalah pembuatan shell sederhana. Shell akan menggunakan read-eval-print-loop (REPL) yang akan menerima perintah dari user dan mengeksekusi perintah tersebut. Pada task ini, kalian diminta untuk mengimplementasikan fungsi `printCWD` yang akan digunakan untuk menampilkan _current working directory_ (CWD) dari shell.

Fungsi `printCWD` akan menerima parameter `byte cwd` yang menunjukkan node index dari _current working directory_. Fungsi akan menampilkan path dari root (`/`) hingga node yang ditunjuk oleh `cwd`. Jika `cwd` adalah `0xFF`, maka path yang ditampilkan adalah `/`. Setiap node yang ditampilkan akan dipisahkan oleh karakter `/`.

### Task 5 - Implementasi parseCommand

Selanjutnya, kalian diminta untuk mengimplementasikan fungsi `parseCommand` yang akan digunakan untuk memisahkan perintah yang diberikan oleh user. Fungsi `parseCommand` akan menerima parameter sebagai berikut.

```c
void parseCommand(char* buf, char* cmd, char arg[2][64]);
```

- `buf` adalah string yang berisi perintah yang diberikan oleh user.
- `cmd` adalah string yang akan digunakan untuk menyimpan perintah yang diberikan oleh user.
- `arg` adalah array of string yang akan digunakan untuk menyimpan argumen dari perintah yang diberikan oleh user.

Karena hanya akan ada 2 argumen yang diberikan oleh user, maka `arg` akan memiliki ukuran 2. Jika argumen yang diberikan oleh user adalah 1, maka `arg[1]` akan berisi string kosong. Jika argumen yang diberikan oleh user adalah 0, maka `arg[0]` dan `arg[1]` akan berisi string kosong.

### Task 6 - Implementasi cd

Fungsi `cd` akan digunakan untuk mengubah _current working directory_ dari shell. Berikut adalah spesifikasi dari fungsi `cd`.

- `cd <dirname>` dapat memindahkan _current working directory_ ke direktori yang berada di bawah _current working directory_.

- `cd ..` akan memindahkan _current working directory_ ke _parent directory_ dari _current working directory_.

- `cd /` akan memindahkan _current working directory_ ke _root directory_.

- `cd` hanya dapat memindahkan _current working directory_ ke direktori, tidak dapat memindahkan _current working directory_ ke file.

- Implementasi relative path dan absolute path tidak diwajibkan.

### Task 7 - Implementasi ls

Fungsi `ls` akan digunakan untuk menampilkan isi dari direktori. Berikut adalah spesifikasi dari fungsi `ls`.

- `ls` akan menampilkan isi dari _current working directory_.

- `ls .` akan menampilkan isi dari _current working directory_.

- `ls <dirname>` akan menampilkan isi dari direktori yang berada di bawah _current working directory_.

- `ls` hanya dapat menampilkan isi dari direktori, tidak dapat menampilkan isi dari file.

- Implementasi relative path dan absolute path tidak diwajibkan.

### Task 8 - Implementasi mv

Fungsi `mv` akan digunakan untuk memindahkan file atau direktori. Berikut adalah spesifikasi dari fungsi `mv`.

- `mv <filename> <dirname>/<outputname>` akan memindahkan file yang berada di bawah _current working directory_ ke direktori yang berada di bawah _current working directory_.

- `mv <filename> /<outputname>` akan memindahkan file yang berada di bawah _current working directory_ ke direktori _root directory_.

- `mv <filename> ../<outputname>` akan memindahkan file yang berada di bawah _current working directory_ ke _parent directory_ dari _current working directory_.

- `mv` hanya dapat memindahkan file, tidak dapat memindahkan direktori.

- Implementasi relative path dan absolute path tidak diwajibkan.

### Task 9 - Implementasi cp

Fungsi `cp` akan digunakan untuk menyalin file. Berikut adalah spesifikasi dari fungsi `cp`.

- `cp <filename> <dirname>/<outputname>` akan menyalin file yang berada di bawah _current working directory_ ke direktori yang berada di bawah _current working directory_.

- `cp <filename> /<outputname>` akan menyalin file yang berada di bawah _current working directory_ ke direktori _root directory_.

- `cp <filename> ../<outputname>` akan menyalin file yang berada di bawah _current working directory_ ke _parent directory_ dari _current working directory_.

- `cp` hanya dapat menyalin file, tidak dapat menyalin direktori.

- Implementasi relative path dan absolute path tidak diwajibkan.

### Task 10 - Implementasi cat

Fungsi `cat` akan digunakan untuk menampilkan isi dari file. Berikut adalah spesifikasi dari fungsi `cat`.

- `cat <filename>` akan menampilkan isi dari file yang berada di bawah _current working directory_.

- Implementasi relative path dan absolute path tidak diwajibkan.

### Task 11 - Implementasi mkdir

Fungsi `mkdir` akan digunakan untuk membuat direktori. Berikut adalah spesifikasi dari fungsi `mkdir`.

- `mkdir <dirname>` akan membuat direktori yang berada di bawah _current working directory_.

## Testing

Untuk melakukan testing, kalian dapat menjalankan `make build run` pada terminal untuk melakukan kompilasi dan menjalankan OS. Setelah itu tutup OS dan jalankan `make generate test=1` untuk melakukan populasi file dan direktori ke dalam filesystem (ubah nilai 1 dengan nomor test yang sesuai). Setelah itu, jalankan kembali OS dengan `make run` dan coba perintah shell yang telah kalian implementasikan.

### `make generate test=1`

Berikut adalah struktur filesystem yang akan digunakan pada test ini.

```
/
├─ dir1
│  ├─ dir1-1
│  │  └─ dir1-1-1
│  └─ dir1-2
│     └─ dirname
├─ dir2
│  └─ dirname
└─ dir3
```

### `make generate test=2`

Berikut adalah struktur filesystem yang akan digunakan pada test ini.

```
/
├─ file-0
├─ dir-1
│  └─ dir-2
│     └─ . . .
│        └─ dir-62
└─ file-63
```

### `make generate test=3`

Berikut adalah struktur filesystem yang akan digunakan pada test ini.

```
/
├─ 1024
├─ 4096
├─ 8192_0
├─ 8192_1
├─ ...
└─ 8192_13
```

### `make generate test=4`

Berikut adalah struktur filesystem yang akan digunakan pada test ini.

```
/
├─ dir1
│  ├─ katanya
│  ├─ dir3
│  │  ├─ bikin
│  │  ├─ fp
│  │  └─ dir4
│  ├─ dir5
│  │  ├─ cuma
│  │  └─ seminggu
├─ dir2
└─ doang
```

## Tips

- Untuk debugging filesystem, kalian dapat mengecek menggunakan hexedit pada Linux atau HxD pada Windows. Dengan informasi sektor map `0x100`, node `0x101` dan `0x102`, serta data `0x103`, kalian dapat mengetahui data yang tersimpan pada filesystem. Untuk mendapatkan offset byte dari sektor, kalian dapat menggunakan rumus `offset = sektor * 512` atau `offset = sektor * 0x200`. Sebagai contoh untuk mengetahui isi dari filesystem map, dapat membuka HxD dan hexedit dengan menekan `Ctrl + G` dan memasukkan offset byte dari sektor map (`0x100 * 0x200 = 0x20000`).

  ![tips-1](./assets/tips-1.png)

- `bcc` tidak memberikan error checking sebanyak `gcc`. Kalian dapat menggunakan `gcc` untuk melakukan error checking pada saat kompilasi.

- Dikarenakan penggunaan `bcc` dengan mode ANSI C, kalian tidak dapat mendeklarasikan variabel di tengah blok kode atau scope. Variabel harus dideklarasikan di awal blok kode atau scope.

- Selalu jalankan `make` pada direktori `praktikum-final`, bukan pada subdirektori.

- Sedikit sneak peek apa yang akan kalian buat.

  https://github.com/sisop-its-s24/praktikum-final/assets/54766683/10055e0d-7a77-44d5-97f5-376bf976d247
