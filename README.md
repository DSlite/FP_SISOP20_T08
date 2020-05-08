# Final Project SISOP 2020 - T08
Penyelesaian Final Project Sistem Operasi 2020

Kelompok T08
  * I Made Dindra Setyadharma (05311840000008)
  * Muhammad Irsyad Ali (05311840000041)

---
## Table of Contents
* [**`clear`**](#command-clear)
* [**`chmod`**](#command-chmod)
* [**`factor`**](#command-factor)
* [**`split`**](#command-split)
* [**`base64`**](#command-base64)
---

## Command Clear
Source Code : https://github.com/DSlite/FP_SISOP20_T08/blob/master/lite/clear.c

### Deskripsi 
Clear merupakan command yang berfungsi untuk membersihkan screen pada terminal 

Program kami pertama-tama akan melakukan pengecekan argumen lalu menggunakan line untuk clear screen pada program c, yang akan membersihkan screen pada terminal. 

### Pembahasan

```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
  if(argc > 1){
    printf(1, "clear: too much arguments");
    exit();
  }
  printf(1, "\e[1;1H\e[2J");
  exit();
}
```

Include beberapa header
* `types.h` berisi tipe data khusus
* `stat.h` berisi struktur data untuk melakukan `fstat`
* `user.h` berisi system_call dan lib yang dapat digunakan pada xv6.

Pertama kami akan melakukan pengecekan jumlah argumen karena Command `clear` hanya menerima satu argumen `(clear)` dengan menggunakan  `if(argc > 1)` dan akan menampilkan error message jika ada argumen lain dan terminal akan di close menggunakan `exit()`.  
  
Kemudian menggunakan `printf(1, "\e[1;1H\e[2J")` dimana `\e[1;1H` akan menaruh kursor pada bagian kiri atas, dan `\e[2J` untuk menghapus semua karakter pada layar.

### Kesulitan  
Tidak ada.

### ScreenShot
![Output](https://user-images.githubusercontent.com/17781660/81388773-5aca3f00-914b-11ea-8e07-0234c3331a59.png)

[Table of Contents](#table-of-contents)

## Command Chmod

Source Code : https://github.com/DSlite/FP_SISOP20_T08/blob/master/lite/chmod.c

### Deskripsi

Command untuk mengganti permission pada sebuah file atau directory.

Contoh penggunaan:
* `chmod 755 file1`
* `chmod 666 file1`

### Pembahasan

Pada filesystem xv6 tidak mengenal `mode` (file permission), sehingga untuk membuat command `chmod`, pertama kita harus mengubah filesystem agar memiliki `mode` dalam struktur inodenya. Lalu xv6 juga tidak memiliki system call untuk mengubah mode tersebut, sehingga harus didefinisikan system call untuk mengubah `mode` tersebut.

#### filesystem

Pertama, kita harus membuat struktur data `mode_t` untuk digunakan pada inode filesystem. mendefinisian `mode_t` terdapat pada [`fs.h`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/fs.h). 

```c
union mode_t {
  struct {
    uint o_x : 1;
    uint o_w : 1;
    uint o_r : 1;
    uint g_x : 1;
    uint g_w : 1;
    uint g_r : 1;
    uint u_x : 1;
    uint u_w : 1;
    uint u_r : 1;
    uint     : 23;
  } flags;
  uint intRep;
};
```

`mode_t` kami definisikan menggunakan `union` untuk memudahkan proses mengubah mode tersebut. Jadi pada code tersebut, ketika kita mengubah `intRep` maka isi dari `flags` juga akan berubah, karena pada `union` seluruh variable menunjuk pada alamat yang sama. variable `intRep` akan digunakan sebagai representasi mode dalam bentuk decimal. lalu variable `flags` digunakan untuk bit value dari masing-masing mode. untuk masing-masing mode akan diassign `1 bit` saja. lalu karena pada `intRep` berupa `unsigned int` maka harus diberikan padding sebanyak `23` bit agar jumlah flags berjumlah `32 bits` (int).

```c
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  union mode_t mode;
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};
```

Lalu kita menambahkan `mode_t` tersebut pada struktur `dinode` (on-disk inode) agar `mode_t` tersebut ada dalam setiap `inode`. Karena kita menambah variable pada `dinode`, maka kita harus mengurangi konstanta `NDIRECT` dengan banyaknya `int` yang kita tambahkan pada `dinode`. Pada kasus ini, maka `NDIRECT` akan diassign menjadi `11`.

```c
#define NDIRECT 11
```

Lalu struct `inode` pada [`file.h`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/file.h) harus ditambahkan dengan `mode_t`.

```c
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count
  struct sleeplock lock; // protects everything below here
  int valid;          // inode has been read from disk?

  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  union mode_t mode;
  uint size;
  uint addrs[NDIRECT+1];
};
```

Lalu kita akan membuat fungsi `chmod` pada filesystem tersebut agar system_call yang nanti akan digunakan memanggil fungsi `chmod` ini. Fungsi ini terletak pada [`fs.c`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/fs.c).

```c
int chmod(struct inode *ip, int mode) {
  if (mode > 511) return -1;
  begin_op();
  ilock(ip);
  ip->mode.intRep = mode;
  iunlock(ip);
  iupdate(ip);
  end_op();
  return 0;
}
```
Fungsi tersebut akan mengecek apakah mode yang diinputkan sudah benar (maximum valuenya 777 (octal) atau dalam decimal `511`). lalu akan menjalankan fungsi `begin_op()` untuk memberitahu pada system akan dilakukan operasi pada filesystem. Lalu `ilock()` untuk mengunci variable `ip` agar tidak terjadi race condition. lalu nilai `ip->mode.intRep` akan diset menjadi `mode`. Setelah itu `ip` akan di `iunlock()` dan di `iupdate()`. dan terakhir akan dijalankan fungsi `end_op()` untuk memberitahu system operasi telah selesai. Fungsi ini nantinya akan dipanggil oleh `sys_chmod`.\

Selain itu kita juga harus menambahkan pada fungsi `ialloc()`, `iupdate()`, dan `stati()` agar memperhatikan nilai `mode` tersebut.

```c
struct inode*
ialloc(uint dev, short type)
{
  int inum;
  struct buf *bp;
  struct dinode *dip;

  for(inum = 1; inum < sb.ninodes; inum++){
    bp = bread(dev, IBLOCK(inum, sb));
    dip = (struct dinode*)bp->data + inum%IPB;
    if(dip->type == 0){  // a free inode
      memset(dip, 0, sizeof(*dip));
      dip->type = type;
      dip->mode.intRep = 493;
      log_write(bp);   // mark it allocated on the disk
      brelse(bp);
      return iget(dev, inum);
    }
    brelse(bp);
  }
  panic("ialloc: no inodes");
}

void
iupdate(struct inode *ip)
{
  struct buf *bp;
  struct dinode *dip;

  bp = bread(ip->dev, IBLOCK(ip->inum, sb));
  dip = (struct dinode*)bp->data + ip->inum%IPB;
  dip->type = ip->type;
  dip->major = ip->major;
  dip->minor = ip->minor;
  dip->nlink = ip->nlink;
  dip->size = ip->size;
  dip->mode.intRep = ip->mode.intRep;
  memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
  log_write(bp);
  brelse(bp);
}

void
stati(struct inode *ip, struct stat *st)
{
  st->dev = ip->dev;
  st->ino = ip->inum;
  st->type = ip->type;
  st->nlink = ip->nlink;
  st->size = ip->size;
  st->mode.intRep = ip->mode.intRep;
}
```

Pada fungsi `stati()` akan mengambil `stat` dari `ip`, sehingga pada [`stat.h`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/stat.h) harus ditambahkan `stat_mode_t` dan struktur `stat` juga harus diubah agar memperhatikan `mode` tersebut.

```c
union stat_mode_t {
  struct {
  uint o_x : 1;
    uint o_w : 1;
    uint o_r : 1;
    uint g_x : 1;
    uint g_w : 1;
    uint g_r : 1;
    uint u_x : 1;
    uint u_w : 1;
    uint u_r : 1;
    uint     : 23;
  } flags;
  uint intRep;
};

struct stat {
  short type;  // Type of file
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short nlink; // Number of links to file
  uint size;   // Size of file in bytes
  union stat_mode_t mode;
};
```

Lalu karena kita menambahkan `mode` tersebut, maka ketika diawal `inode` tersebut dibuat harus memiliki default value untuk `mode`nya tersebut. Maka kita harus mengganti fungsi `inode` pada [`mkfs.c`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/mkfs.c) juga.

```c
uint
ialloc(ushort type)
{
  uint inum = freeinode++;
  struct dinode din;

  bzero(&din, sizeof(din));
  din.type = xshort(type);
  din.nlink = xshort(1);
  din.size = xint(0);
  din.mode.intRep = 493;
  winode(inum, &din);
  return inum;
}
```

Pada code tersebut, default tiap kali sebuah inode dialokasikan memiliki permision `rwxr-xr-x`, sehingga pada `din.mode.intRep` akan diset menjadi `493` (atau dalam octal menjadi `755`). Setelah pendefinisian `mode` pada filesystem selesai, kita dapat membuat system_call `chmod` untuk mengganti `mode` tersebut.

### sys_chmod

Ketika user memanggil sebuah system_call, maka system_call tersebut akan dipanggil melalui [`user.h`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/user.h). Sehingga fungsi system_call `chmod` perlu didefinisikan pada [`user.h`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/mkfs.c).

```c
int chmod(char *, int);
```

Lalu user.h tersebut akan mencari system_call yang sesuai pada [`usys.S`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/usys.S). Sehingga system_call `chmod` perlu didefinisikan pada file tersebut.

```c
SYSCALL(chmod)
```

Dari `usys.S` ini akan mencari syscall yang telah didefinisikan pada [`syscall.h`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/syscall.h). Maka `chmod` perlu didefiniskan juga disana.

```c
#define SYS_chmod  22
```

Nilai `22` disini mengikuti pada SYS_call sebelumnya. Lalu disini akan mencari `SYS_chmod` pada [`syscall.c`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/syscall.c), maka sys_chmod tersebut akan didefinisikan pada file tersebut.

```c
extern int sys_chmod(void);

static int (*syscalls[])(void) = {
    ...
    [SYS_chmod]   sys_chmod,
}
```

Lalu dari sana akan memanggil fungsi system_call `sys_chmod` yang sebenarnya. Fungsi tersebut akan didefinisikan pada [`sysfile.c`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/sysfile.c).

```c
int
sys_chmod(void)
{
  char *pathname;
  int mode;
  if (argstr(0, &pathname) < 0) return -1;
  if (argint(1, &mode) < 0) return -1;
  struct inode *ip = namei(pathname);
  if (!ip) return -1;
  return chmod(ip, mode);
}
```

Pada code tersebut, akan mengambil `pathname` pada argument ke-0 dan `mode` pada argument ke-1. Setelah itu akan dibuat pointer `ip` yang menunjuk inode dari nama `pathname` tersebut menggunakan fungsi `namei()`. Setelah itu pointer `ip` tersebut akan dicek dan akan dijalankan fungsi `chmod()` yang telah dibuat pada [`fs.c`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/fs.c). fungsi `chmod()` ini harus didefinisikan pada [`defs.c`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/defs.c) agar dapat dipanggil pada [`sysfile.c`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/sysfile.c).

```c
int             chmod(struct inode*, int);
```

Lalu system call `chmod` tersebut dapat dipanggil pada program biasa.

### chmod

Untuk membuat user command `chmod` hanya perlu memanggil system_call `chmod()` yang telah didefinisikan pada [`user.h`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/user.h).

```c
#include "types.h"
#include "stat.h"
#include "user.h"

int checkInt(char *arg) {
  for (int n = 0; arg[n]; n++) {
    if (arg[n] < '0' || arg[n] > '9') return 0;
  }
  return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf(1, "chmod: wrong arguments\n");
        exit();
    }
    if (strlen(argv[1]) != 3) {
        printf(1, "chmod: wrong mode\n");
        exit();
    }
    if (checkInt(argv[1]) == 0) {
        printf(1, "chmod: wrong mode\n");
        exit();
    }
    int modeInt = atoi(argv[1]);
    int other = modeInt%10;
    int group = (modeInt/10)%10;
    int user = (modeInt/100)%10;
    if (user > 7 || group > 7 || other > 7) {
        printf(1, "chmod: wrong mode\n");
        exit();
    }
    int mode = (user << 6) + (group << 3) + other;
    if (chmod(argv[2], mode)) {
        printf(1, "chmod: failed to change permission\n");
    }
    exit();
}
```

Pertama arguments akan dicek apakah sesuai dengan ketentuan. 
* command yang diinputkan `chmod MODE FILE`
* `MODE` memiliki panjang 3.
* `MODE` tidak memiliki karakter.
untuk mengecek `mode` yang diinputkan, disini kami menggunakan fungsi `checkInt()` untuk mengecek apakah terdapat karakter pada variable yang diinputkan. Lalu setelah argument dicek, maka `mode` akan dibagi berdasarkan user, group, other yang sesuai. Lalu dicek untuk masing-masing value mode tersebut maksimal `7`. Lalu masing-permission tersebut akan dijadikan satu dengan bitwise operation (Jadi user menempati 3 bit pertama, group 3 bit ditengah, dan other 3 bit terakhir). Lalu system_call `chmod()` akan dipanggil sesuai `path` dan `mode` yang diminta. Jika gagal maka akan menampilkan output gagal.\

User command untuk melakukan `chmod` sudah bisa dijalankan. Untuk melihat perubahan `mode` (permission) tersebut, kelompok kami mengedit command [`ls.c`](https://github.com/DSlite/FP_SISOP20_T08/blob/master/ls.c) agar menampilkan permission masing-masing entry yang ditampilkan dengan melihat `stat` dari entry tersebut (karena `stat` tadi sudah diganti agar memiliki `mode`).

### Kesulitan  
Pendefinisian system_call untuk chmod, dan juga filesystemnya

### ScreenShot

![Output](https://user-images.githubusercontent.com/17781660/81389672-cbbe2680-914c-11ea-9c4d-3f1f041e35e1.png)
![Output](https://user-images.githubusercontent.com/17781660/81389792-f314f380-914c-11ea-8eac-94ab610a0d78.png)

[Table of Contents](#table-of-contents)


## Command Factor
Source Code : https://github.com/DSlite/FP_SISOP20_T08/blob/master/lite/factor.c

### Deskripsi:

Command ini digunakan untuk menampilkan faktorisasi prima dari suatu bilangan yang diberikan

Contoh Penggunaan: 
* `factor 10`
* `factor 2 3 5 10`

### Pembahasan:

```c
#include "types.h"
#include "stat.h"
#include "user.h"
```

Include beberapa header
* `types.h` berisi tipe data khusus
* `stat.h` berisi struktur data untuk melakukan `fstat`
* `user.h` berisi system_call dan lib yang dapat digunakan pada xv6.

```c
int integerSqrt(int n) {
  int i = 1;
  while(i*i<=n) {
    i++;  
  }
  return --i;
}
```
Fungsi yang akan mereturn hasil akar dari argumen yang di berikan dalam bentuk `int`.
  
`while()` loop disini akan berjalan selama `i` * `i` <= dari `n` dan `i` akan di increment. Setelah selesai maka nilai `i` akan di decrement (karena `i` akan lebih 1 loop) dan di return oleh fungsi ini sehingga mendapatkan **akar `int`** dari `n`.

```c
void printFactor(int n) {
  while(n%2 == 0) {
    printf(1, " %d", 2);
    n /= 2;
  }
  int end = integerSqrt(n);
  for (int i = 3; i <= end; i += 2) {
    while (n%i == 0) {
      printf(1, " %d", i);
      n/=i;
    } 
  }
  if (n > 2) { 
    printf(1, " %d", n);
  }
}
```
Fungsi untuk menampilkan faktorisasi prima dari argumen yang diberikan 

`While()` loop pertama akan berjalan untuk setiap `n` yang habis dibagi 2, dan akan menampilkan `" 2"` pada terminal, kemudian `n` akan dibagi 2.

Selanjutnya adalah pendefinisian variabel `end` yang akan diisi hasil akar dari `n` menggunakan fungsi `integerSqrt()`. `for()` disini akan berjalan untuk `i` yang <= dari `end` dan `i` akan ditambah `2` setiap loop. untuk masing-masing `i` akan dicek apakah selagi `i` dapat habis membagi `n`. Jika iya maka `i` akan di print dan `n` akan dibagi dengan `n`. Lalu setelah semua `i` dicek, maka `n` akhir akan dicek apakah lebih besar 2, jika iya maka print nilai `n` tersebut.

Referensi: [GeeksForGeeks](https://www.geeksforgeeks.org/print-all-prime-factors-of-a-given-number/).

```c
int checkInt(char *arg) {
  for (int n = 0; arg[n]; n++) {
    if (arg[n] < '0' || arg[n] > '9') return 0;
  }
  return 1;
}
```
Fungsi untuk melakukan penegcekan pada `arg` apakah terdapat selain karakter **numeric**.

Pertama adalah `for()` loop dengan variabel `n` yang akan meng-interasi semua argumen sampai habis menggunakan `for (int n = 0; arg[n]; n++)`. Jika pada salah satu karakter pada `arg` bukan **numeric** maka akan meng-return `0`. Jika tidak ada akan meng-return `1`. 

```c
int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (checkInt(argv[i]) == 0) {
      printf(2, "factor: cannot calculate %s\n", argv[i]);
    } else {
      int n = atoi(argv[i]);
      printf(1, "%d:", n);
      printFactor(n);
      printf(1, "\n");
    }
  }
  exit();
}
```

Pada bagian main kami membuat `for()` loop untuk mengiterasi tiap `argv`, kemudian akan di lakukan pengecekan `argv` tersebut menggunakan fungsi `chechkInt` pada setiap argumen dan akan menampilkan error jika value yang di return `0`.

Untuk argumen selain diatas maka setiap argumennya akan dirubah menjadi `int` value menggunakan fungsi `atoi()` di simpan pada variabel `n`, kemudian akan dipanggil fungsi `printFactor()` terhadap `n` untuk menampilkan faktorisasi prima dari `n`.

### Kesulitan
Tidak ada.

### ScreenShot
![Output](https://user-images.githubusercontent.com/17781660/81385005-323f4680-9145-11ea-934e-c0bfa4614cee.png)

[Table of Contents](#table-of-contents)

## Command Split
Source Code : https://github.com/DSlite/FP_SISOP20_T08/blob/master/lite/split.c

### Deskripsi

Command split digunakan untuk membagi sebuah file berdasarkan jumlah line menjadi beberapa parts.

Command ini akan menggunakan prefix dan suffix untuk membagi partnya, suffix disini menggunakan alphabet dan akan di"increment" pada setiap part. Pada command ini terdapat beberapa opsi.
* `-l` untuk mendefinisikan maksimum line pada satu part (Default: 1000)
* `-a` untuk mendefinisikan maksimum suffix yang diinginkan (Default: 2)

Contoh penggunaan:
* `split file1`
* `split -l 5 -a 3 file1 prefix-`

### Pembahasan
```c
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define DEFAULT_PREFIX "x"
#define DEFAULT_LINE 1000
#define DEFAULT_NUM_SUFFIX 2

char *default_suffix = "abcdefghijklmnopqrstuvwxyz";
```
Include beberapa header
* `types.h` berisi tipe data khusus
* `stat.h` berisi struktur data untuk melakukan `fstat`
* `user.h` berisi system_call dan lib yang dapat digunakan pada xv6.
* `fcntl.h` berisi operasi file yang dapat dilakukan ketika membuka sebuah file.

kemudian dilanjutkan oleh pendefinisian untuk 
* `DEFAULT_PREFIX` dengan nilai default prefixnya: `"x"`
* `DEFAULT_LINE` dengan nilai default maximum line: `1000`
* `DEFAULT_NUM_SUFFIX` dengan nilai default banyaknya suffix: `2`  

kemudian dilanjutkan pendefinisian `default_suffix` sebagai list suffix yang akan digunakan.

```c
int checkInt(char *arg) {
  for (int n = 0; arg[n]; n++) {
    if (arg[n] < '0' || arg[n] > '9') return 0;
  }
  return 1;
}
```
Pertama kami mendefinisikan fungsi `checkInt()`. Disini fungsinya sama pada command `factor` yaitu untuk mengecek `arg` apakah hanya karakter **numeric** saja atau tidak.

#### Fungsi nextSuffix

```c
int nextSuffix(char *suffix) {
    int carrier = 1;
    int suffix_len = strlen(suffix);
    for (int i = suffix_len-1; i>=0; i--) {
        if (carrier == 1) {
            carrier = 0;
            char *cursor = strchr(default_suffix, *(suffix+i))+1;
            if (cursor-default_suffix == strlen(default_suffix)) {
                cursor = default_suffix;
                carrier = 1;
            }
            *(suffix+i) = *cursor;
        }
    }
    return carrier;
}
```

Fungsi ini digunakan untuk mengubah suffix menjadi suffix berikutnya, terdapat variable `carrier` yang selama nilainya `1` maka suffix akan "dijumlah", untuk karakter `z`
pada karakter suffix yang terakhir maka akan kembali ke awal contoh: xaz menjadi xba. Fungsi ini akan mereturn `1` jika suffix sudah penuh atau mencapai karakter `z...z`. Jika fungsi ini mereturn `1` maka telah tercapai suffix terakhir.

#### Fungsi split

```c
void split(int fd, int max_line, int max_suffix, char *prefix) {
    char suffix[max_suffix];
    for (int i = 0; i < max_suffix; i++) {
        suffix[i] = *default_suffix;
    }
```

Fungsi ini digunakan untuk melakukan split terhadap `fd` dengan `max_line`, `max_suffix`, dan `prefix` yang diinputkan. Pertama pendefinisian buff `suffix`  dengan `max_suffix`. `for()` loop disini akan berjalan sebanyak `max_suffix` untuk menset nilai awal `suffix` menjadi karakter pertama pada `default_suffix`.

```c
    int fd_parts;
    char path_parts[1000];
    strcpy(path_parts, prefix);
    strcpy(path_parts+strlen(prefix), suffix);
    fd_parts = open(path_parts, O_WRONLY | O_CREATE);

    char buf[1024];
    int num_line = 0;
    int n;
```

Pendefinisan variabel `fd_parts` untuk menyimpan splitted file yang akan diuat, buffer `path_parts` yang akan berisi gabungan dari prefix dengan suffixnya. Contoh default: xaa. Kemudian file akan dibuat dengan nama sesuai `path_parts` menggunakan `open(path_parts, O_WRONLY | O_CREATE)` dan di simpan kedalam file descriptor `fd_parts`.

```c
    while((n = read(fd, buf, 1024)) > 0) {
        char *pa = buf;
        char *pb;
        while ((pb = strchr(pa, '\n')) > 0) {
            write(fd_parts, pa, pb-pa+1);
            if (++num_line == max_line) {
                 num_line = 0;
                close(fd_parts);
                if (nextSuffix(suffix) == 1) {
                    printf(2, "split: not enough suffix\n");
                    exit();
                }
                strcpy(path_parts+strlen(prefix), suffix);
                fd_parts = open(path_parts, O_WRONLY | O_CREATE);
            }
            pa = pb+1;
        }
        if (pa-buf < strlen(buf)) {
            write(fd_parts, pa, strlen(pa));
        }
    }
    close(fd_parts);
}
```

`while()` loop pertama digunakan untuk melakukan membaca tiap 1024 karakter pada `fd` dan karakter tersebut disimpan dalam variable `buf`. Kemudian akan diset pointer `pa` yang menunjuk pada `buf` dan pointer `pb`.

Kemudian `while()` loop kedua akan melakukan iterasi selama menemukan `\n` (pointer yang menunjuk ke `\n` akan disimpan pada variable `*pb`) akan dilakukan beberapa operasi. Pertama akan writing ke `fd_parts` dari `*pa` sampai karakter yang ditunjuk `*pb` (`\n`).

Kemudian akan dicek jika `num_line` yang telah di-increment sama dengan `max_line`. Jika sudah sama, maka suffix perlu diganti menjadi suffix berikutnya. Step pertama adalah mereset nilai `num_line`. lalu `fd_parts` di `close()`. Lalu akan dijalankan fungsi `nextSuffix()` pada `suffix`, jika yang direturn merupakan `1` (sudah mencapai maximum suffix), maka akan diprint error dan program langsung di `exit(0)`. Jika suffix tersebut masih dapat di `nextSuffix()`, maka akan dibuat path yang mengarah ke `path_parts` sesuai dengan `suffix` yang baru. Lalu `path_parts` tersebut akan diopen kembali dan disimpan pada `fd_parts`. Lalu `*pa` akan diset menjadi pointer `*pb+1` (satu karakter setelah karakter yang ditunjuk `pb`);

Kemudian jika `pa-buf < strlen(buf)` maka `pa` akan di `write()` kedalam `fd_parts` yang dengan panjang `pa`. Hal tersebut dilakukan untuk mengwrite string terakhir yang tidak masuk dalam `while()` loop. Lalu setelah semua karakter pada `fd` telah terbaca, maka `fd_parts` akan di `close()`.


#### Fungsi main

```c
int main(int argc, char *argv[]) {
    char prefix[1000];
    int fd;
    int max_line = DEFAULT_LINE;
    int max_suffix = DEFAULT_NUM_SUFFIX;
    strcpy(prefix, DEFAULT_PREFIX);
    int pos=argc;
```

Pada `main()`, akan di set buffer untuk `prefix`, serta variabel untuk file descriptor `fd`. Disini kami juga mendefinsikan max_line, prefix dan suffix yang akan dibuat sebagai default 
terlebih dahulu. 
Kemudian ada pendefinisian variabel `pos` yang menyimpan banyak argumen 

```c
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            if (i+1 >= argc) {
                printf(2, "split: wrong arguments\n");
                exit();
            }
            if (checkInt(argv[i+1]) == 0) {
                printf(2, "split: wrong arguments\n");
                exit();
            }

            max_suffix = atoi(argv[++i]);

```
Bagian ini adalah pengecekan pada setiap argumen dari awal sampai akhir untuk karakter `-a` yang akan men-set suffix maksimal sesuai dengan argumen yang diinputkan setelah 
karakter `-a` dengan menggunakan  `max_suffix = atoi(argv[++i])` jika argumen telah benar

```c
        } else if (strcmp(argv[i], "-l") == 0) {
            if (i+1 >= argc) {
                printf(2, "split: wrong arguments\n");
                exit();
            }
            if (checkInt(argv[i+1]) == 0) {
                printf(2, "split: wrong arguments\n");
                exit();
            }

            max_line = atoi(argv[++i]);
        } else {
            pos = i;
            break;
        }
    }
```
Bagian ini adalah pengecekan pada setiap argumen dari awal sampai akhir untuk karakter `-l` yang akan men-set maxline sesuai dengan argumen yang diinputkan setelah 
karakter `-l` dengan menggunakan  `max_suffix = atoi(argv[++i])`

`else` kedua disini adalah jika tidak ada argumen `-a` atau `-l` maka akan index ke-`i` akan digunakan sebagai `path`

```c
    if (pos+1 < argc) {
        memset(prefix, 0, 1000);
        strcpy(prefix, argv[pos+1]);
    }

    fd = open(argv[pos], O_RDONLY);
    if (fd < 0) {
        printf(2, "split: cannot open\n", argv[pos]);
        exit();
```

Bagian ini berfungsi untuk membuat dan men-set prefix sesuai dengan `argv[pos+1]` yang merupakan `prefix` setelah nama file 

Kemudian file yang diinputkan akan dibuka dan di set kedalam `fd` menggunakan `open(argv[pos], O_RDONLY)` lalu akan ditampilkan error message jika file tidak dapat dibuka atau `if (fd < 0)` dan akan di `exit(0)`

```c
    } else {
        struct stat st;
        if (fstat(fd, &st) < 0) {
            printf(2, "split: cannot stat\n", argv[pos]);
            close(fd);
            exit();
        }
        if (st.type != T_FILE) {
            printf(2, "split: cannot split\n", argv[pos]);
            close(fd);
            exit();
        }
        split(fd, max_line, max_suffix, prefix);

        close(fd);
        exit();
    }
    
    exit();
}
```

Bagian ini untuk `fd` yang dapat dibuka maka akan dilakukan pengecekan untuk stat dan bentuk file menggunakan ` if (fstat(fd, &st) < 0)` dan `if (st.type != T_FILE)`jika tidak bisa di stat atau bentuk bukan file reguler maka akan ditampilkan error dan akan di `exit(0)`

Fungsi split akan dijalankan pada `fd` dengan menggunakan `max_line, max_suffix, prefix` seusai dengan input (atau default) jika tidak ada, selanjutnya `fd` akan diclose dan di `exit()`.

### Kesulitan 
Tidak ada. 

### ScreenShot  
![Output](https://user-images.githubusercontent.com/17781660/81388148-6ec17100-914a-11ea-8c7d-c60b501f1409.png)
![Output](https://user-images.githubusercontent.com/17781660/81388187-826cd780-914a-11ea-8fd4-7c42ddfab03e.png)
![Output](https://user-images.githubusercontent.com/17781660/81388406-d4156200-914a-11ea-9c04-7dbfc0d6bb93.png)

[Table of Contents](#table-of-contents)

## Command Base64
Source Code : https://github.com/DSlite/FP_SISOP20_T08/blob/master/lite/base64.c

### Deskripsi 
Base64 merupakan command yang dapat mengubah data menjadi bentuk base64, command ini akan menampilkan hasil encode pada sebuah file yang dituju atau pada `stdin`.

Command ini akan membaca tiap 3 karakter (1 karakter = 8 bit) dan mengubahnya menjadi 4 karakter (1 karakter = 6 bit) base64 sesuai dengan keynya. Disini juga akan mengimplimentasikan padding jika karakter yang diinput tidak habis dibagi 3.

Contoh penggunaan:
* `base64 file1`
* `echo test | base64` 

### Pembahasan

```c
#include "types.h"
#include "stat.h"
#include "user.h"

char *key = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+Base64"
```

Include beberapa header
* `types.h` berisi tipe data khusus
* `stat.h` berisi struktur data untuk melakukan `fstat`
* `user.h` berisi system_call dan lib yang dapat digunakan pada xv6.

Pertama kami mendefinisikan key untuk encoding dengan menggunakan variable pointer `key` .

### Fungsi Encode
```c
void encode (int fd) {
  char buf[3];
  int n;
  while((n = read(fd, buf, 3)) > 0){
    int bits = 0;

    for (int i = 0; i < n; i++) {
      bits = bits << 8;
      bits += buf[i];
    }
```
Pada fungsi `encode`, kami membuat buffer `char buf[3]` berfungsi untuk menyimpan 3 bytes karakter yang akan dibaca oleh fungsi `read()` dan `n` sebagai variable untuk menyimpan banyaknya karakter yang di `read()` pada file descriptor dan sebagai parameter `while`

Selanjutnya `while` loop disini digunakan untuk membaca setiap 3 karakter pada `fd` dan memasukkannya kedalam `buf`. Lalu didefinisikan variable `bits` untuk menyimpan ketiga karakter tersebut dalam bentuk `int`.

lalu `for` loop untuk memasukkan masing-masing karakter pada `buf` menjadi `int` sesuai posisinya dalam `bits`.

```c
    if (n == 1) {
      bits = bits << 4;
      printf(1, "%c", key[(bits >> 6) & 63]);
      printf(1, "%c", key[(bits) & 63]);
      printf(1, "==");
    } else if (n == 2) {
      bits = bits << 2;
      printf(1, "%c", key[(bits >> 12) & 63]);
      printf(1, "%c", key[(bits >> 6) & 63]);
      printf(1, "%c", key[(bits) & 63]);
      printf(1, "=");
    } else if (n == 3) {
      printf(1, "%c", key[(bits >> 18) & 63]);
      printf(1, "%c", key[(bits >> 12) & 63]);
      printf(1, "%c", key[(bits >> 6) & 63]);
      printf(1, "%c", key[(bits) & 63]);
    }

  }   
  printf(1, "\n");
}
```
Pada bagian ini terdapat tiga kondisi yaitu untuk `n == 1`, `n == 2` atau `n == 3` untuk masing-masing kemungkinan banyaknya karakter yang di `read`.

Untuk `n == 1` akan diappend bits `0` sebanyak 4 agar panjang variable `bits` kelipatan dari 6 `bit`, sehingga dilakukan left-shift sebanyak 4. Lalu akan terdapat dua karakter pada `bits`, sehingga diambil setiap 6 `bit` dan dicari posisinya pada `key`. Karena hanya menjadi 2 karakter saja, akan dipadding dengan `"=="`.

Untuk `n == 2` akan diappend bits `0` sebanyak 2 agar panjang variable `bits` kelipatan dari 6 `bit`, sehingga dilakukan left-shift sebanyak2. Lalu akan terdapat 3 karakter pada `bits` sehingga diambil setiap 6 `bit` dan dicari posisinya pada `key`. Karena hanya menjadi 3 karakter saja, akan dipadding dengan `"="`.

Untuk `n == 3`, karena sudah panjang karakter sudah kelipatan 6 (24 bits), maka tiap 6 `bit` akan diubah menjadi karakter menggunakan `key`.

```c
int main(int argc, char *argv[]) {
  if (argc == 1) {
    encode(0);
    exit();
  }

  int fd;
```

Pada bagian main, pertama akan dilakukan dulu pengecekan jumlah argumen yang diinputkan. Untuk `if (argc == 1)`, akan dilakukan `encode()` terhadap `stdin`. Setelah di-`encode()` akan di-`exit()`.

Untuk yang memiliki arguments maka:
```c
  for (int i = 1; i < argc; i++) {
    fd = open(argv[i], 0);
    if (fd < 0) {
      printf(2, "base64: cannot open %s\n", argv[i]);
    } else {
      struct stat st;
      if (fstat(fd, &st) < 0) {
        printf(2, "base64: cannot stat %s\n", argv[i]);
        close(fd);
        continue;
      }
      if (st.type != T_FILE) {
        printf(2, "base64: cannot encode %s\n", argv[i]);
        close(fd);
        continue;
      }
      printf(1, "%s: ", argv[i]);
      encode(fd);
      close(fd);
    }
  }
  exit();
}
```
Untuk arguments lebih dari satu akan ada `for()` loop yang berjalan sebanyak argumen yang diinputkan.  

Pertama tiap file yang diinputkan akan di buka dan distore kedalam file descriptor `fd` menggunakan `fd = open(argv[i], 0)`, selanjutnya untuk `fd` yang gagal dibuka (filenya tidak ada) akan ditampilkan error `printf(2, "base64: cannot open %s\n", argv[i])` dan akan di `continue()` untuk pengecekan `fd` selanjutnya.  

Untuk `fd` yang ada isinya namun gagal di-`fstat()` maka akan ditampilkan error `printf(2, "base64: cannot stat %s\n", argv[i])` dan akan di `continue()` untuk pengecekan `fd` selanjutnya.    
  
Untuk `fd` yang ada isinya dan bisa di `stat` namum bukan merupakan file, maka akan ditamplikan error `printf(2, "base64: cannot encode %s\n", argv[i])` dan akan di `continue()` untuk pengecekan `fd` selanjutnya.

Jika semua kondisi tersebut dapat dilewati, maka `fd` akan di `encode()` dan akan langsung ditampilkan pada `stdout`. Lalu `fd` akan di `close()`. Setelah semua argument telah terbaca, maka program akan di `exit()`.

### Kesulitan
Tidak ada.

### ScreenShot
![Output](https://user-images.githubusercontent.com/17781660/81383168-29994100-9142-11ea-86db-1324781ef525.png)
![Output](https://user-images.githubusercontent.com/17781660/81383534-c65bde80-9142-11ea-9abe-eb580ba0f295.png)

[Table of Contents](#table-of-contents)
