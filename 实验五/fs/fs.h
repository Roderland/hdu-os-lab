//
// Created by roderland on 2020/12/23.
//

#ifndef FS_FS_H
#define FS_FS_H

#define LOCAL_ADDRESS "./tmp"
#define BLOCK_SIZE 512
#define BLOCK_NUM 4096
#define BITMAP_SIZE 128
#define DENTRY_SIZE 32
#define FILENAME_LEN 28
#define DIRECT_INODE_NUM 10
#define SHIFT 5
#define MASK 0x1F
#define INDEX_NUM 16
#define PWD_LEN 128
#define MAX_OPEN 8
#define MAX_DEPTH 8
//#define BUFFER_SIZE 1024

struct dentry {
    int ino;// inode block no
    char name[FILENAME_LEN];// file or dir name
};

struct inode {
    int ino;// inode block no
    char name[FILENAME_LEN];// file or dir name
    int type;// file type (dir:0, file:1)
    int size;// file size
    int blocks;// file_data used block num
    int pno;// parent dir_data block no
    int direct[DIRECT_INODE_NUM];// direct file_data block no
    int indirect1;// index_data block no
    int indirect2;
    int indirect3;
};

struct index_data {
    int bno[INDEX_NUM];
};

struct dir_data {
    // 0 is .
    // 1 is ..
    // 2
    struct dentry entry[BLOCK_SIZE/DENTRY_SIZE];
};

struct file_data {
    char content[BLOCK_SIZE];
};

struct bitmap_data {
    int bitmap[BITMAP_SIZE];
};

struct useropen {
    int fd;
    int offset;
    char name[FILENAME_LEN];
    char pwd[PWD_LEN];
};

void my_format();

void my_mkdir();

void my_rmdir();

void my_ls();

void my_cd(char *name);

void my_create();

void my_open();

void my_close();

void my_write();

void my_read();

void my_rm();

void my_exitsys();

int bitmap[BLOCK_NUM / 4];
/* a[i>>SHIFT]是第i位应该在第几个int上 */
/* (1<<(i & MASK))是第i位在该int上的第几个bit */
void set(int i) {
    bitmap[i >> SHIFT] |= (1 << (i & MASK));
}

void clr(int i) {
    bitmap[i >> SHIFT] &= ~(1 << (i & MASK));
}

int check(int i) {
    return bitmap[i >> SHIFT] & (1 << (i & MASK));
}

#endif //FS_FS_H
