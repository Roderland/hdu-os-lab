#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

void *disk[BLOCK_NUM];
char type[BLOCK_NUM][8];
struct dir_data *cur_dir;
char buffer[BLOCK_SIZE + 1];
char command[64];
struct useropen useropen[MAX_OPEN];
char pwd[PWD_LEN];
int open_num;

void my_format() {
    FILE *fp;
    if ((fp = fopen(LOCAL_ADDRESS, "rb")) == NULL) {
        puts("Fail to open file!");
        exit(0);
    }
    rewind(fp);
    for (int i = 0; i < BLOCK_NUM; ++i) {
        if (fscanf(fp, "%s ", type[i]) < 0) break;
        if (strcmp(type[i], "dir") == 0) {
            struct dir_data *dir = malloc(BLOCK_SIZE);
            int j = 0;
            while (fscanf(fp, "%d %s ", &dir->entry[j].ino, dir->entry[j].name) > 0) {
                j++;
            }
            for (int k = j; k < BLOCK_SIZE / DENTRY_SIZE; ++k) {
                dir->entry[k].ino = -1;
            }
            disk[i] = dir;
        } else if (strcmp(type[i], "file") == 0) {
            char ch;
            struct file_data *file = malloc(BLOCK_SIZE);
            int j = 0;
            while ((ch = fgetc(fp)) != '$' && j < BLOCK_SIZE) {
                file->content[j++] = ch;
            }
            disk[i] = file;
        } else if (strcmp(type[i], "inode") == 0) {
            struct inode *inode = malloc(BLOCK_SIZE);
            fscanf(fp, "%d %s %d %d %d %d", &inode->ino, inode->name, &inode->type, &inode->size, &inode->blocks,
                   &inode->pno);
            for (int j = 0; j < DIRECT_INODE_NUM; ++j) {
                fscanf(fp, "%d", &inode->direct[j]);
            }
            fscanf(fp, "%d %d %d", &inode->indirect1, &inode->indirect2, &inode->indirect3);
            disk[i] = inode;
        } else if (strcmp(type[i], "bitmap") == 0) {
            struct bitmap_data *bitmapData = malloc(BLOCK_SIZE);
            for (int j = 0; j < BITMAP_SIZE; ++j) {
                fscanf(fp, "%d", &bitmapData->bitmap[j]);
            }
            disk[i] = bitmapData;
        } else if (strcmp(type[i], "empty") == 0) {
            disk[i] = NULL;
        }
    }
    fclose(fp);
    cur_dir = disk[0];
    buffer[0] = '\0';
    command[0] = '\0';
    open_num = 0;
}

void my_exitsys() {
    FILE *fp;
    if ((fp = fopen("./disk", "wb")) == NULL) {
        puts("Fail to open file!");
        exit(0);
    }
    rewind(fp);
    struct dir_data *root = disk[0];
    fprintf(fp, "dir ");
    for (int i = 0; i < BLOCK_SIZE / DENTRY_SIZE && root->entry[i].ino >= 0; ++i) {
        fprintf(fp, "%d %s ", root->entry[i].ino, root->entry[i].name);
    }
    struct bitmap_data *bitmapData = disk[1];
    fprintf(fp, "\nbitmap ");
    for (int i = 0; i < BITMAP_SIZE; ++i) {
        fprintf(fp, "%d ", bitmapData->bitmap[i]);
    }
    for (int i = 2; i < BLOCK_NUM; ++i) {
        if (strcmp(type[i], "dir") == 0) {
            struct dir_data *dirData = disk[i];
            fprintf(fp, "\ndir ");
            for (int j = 0; j < BLOCK_SIZE / DENTRY_SIZE && dirData->entry[j].ino >= 0; ++j) {
                fprintf(fp, "%d %s ", dirData->entry[j].ino, dirData->entry[j].name);
            }
        } else if (strcmp(type[i], "file") == 0) {
            struct file_data *fileData = disk[i];
            fprintf(fp, "\nfile %s$ ", fileData->content);
        } else if (strcmp(type[i], "inode") == 0) {
            struct inode *inode = disk[i];
            fprintf(fp, "\ninode %d %s %d %d %d %d ", inode->ino, inode->name, inode->type, inode->size, inode->blocks,
                    inode->pno);
            for (int j = 0; j < DIRECT_INODE_NUM; ++j) {
                fprintf(fp, "%d ", inode->direct[j]);
            }
            fprintf(fp, "%d %d %d ", inode->indirect1, inode->indirect2, inode->indirect3);
        } else if (strcmp(type[i], "index") == 0) {
            struct index_data *indexData = disk[i];
            fprintf(fp, "\nindex ");
            for (int j = 0; j < INDEX_NUM && indexData->bno[j] >= 0; ++j) {
                fprintf(fp, "%d ", indexData->bno[j]);
            }
        } else if (strcmp(type[i], "empty") == 0) {
            fprintf(fp, "\nempty");
        }
    }
    fclose(fp);
}

void my_ls() {
    int flag = 0;
    for (int i = 2; i < BLOCK_SIZE / DENTRY_SIZE && cur_dir->entry[i].ino >= 0; ++i) {
        printf("%s ", cur_dir->entry[i].name);
        flag = 1;
    }
    if (flag) printf("\n");
}

void my_cd(char *name) {
    if (strcmp(name, ".") == 0) return;
    else if (strcmp(name, "..") == 0) {
        struct dir_data *dir = disk[cur_dir->entry[1].ino];
        cur_dir = dir;
        return;
    }
    for (int i = 2; i < BLOCK_SIZE / DENTRY_SIZE && cur_dir->entry[i].ino >= 0; ++i) {
        if (strcmp(name, cur_dir->entry[i].name) == 0) {
            int ino = cur_dir->entry[i].ino;
            //printf("%s %d ", cur_dir->entry[i].name,  ino);
            struct inode *inode = disk[ino];
            if (inode->type == 0) {
                cur_dir = (struct dir_data *) disk[inode->direct[0]];
            } else {
                printf("%s is a file!\n", name);
            }
            return;
        }
    }
    printf("%s is not exist!\n", name);
}

void my_mkdir(char *name) {
    int i = 2;
    for (; i < BLOCK_SIZE / DENTRY_SIZE && cur_dir->entry[i].ino >= 0; ++i) {
        if (strcmp(name, cur_dir->entry[i].name) == 0) {
            printf("%s is already exist!\n", name);
            return;
        }
    }
    if (i == BLOCK_SIZE / DENTRY_SIZE) {
        printf("current directory is full!\n");
        return;
    }
    for (int j = 2; j < BLOCK_NUM; ++j) {
        if (strcmp(type[j], "empty") == 0) {
            for (int k = j + 1; k < BLOCK_NUM; ++k) {
                if (strcmp(type[k], "empty") == 0) {
                    struct dir_data *dirData = malloc(BLOCK_SIZE);
                    struct inode *inode = malloc(BLOCK_SIZE);
                    // j - dir_data
                    dirData->entry[0].ino = k;
                    strcpy(dirData->entry[0].name, name);
                    dirData->entry[1].ino = cur_dir->entry[0].ino;
                    strcpy(dirData->entry[1].name, cur_dir->entry[0].name);
                    for (int l = 2; l < BLOCK_SIZE / DENTRY_SIZE; ++l) {
                        dirData->entry[l].ino = -1;
                        dirData->entry[l].name[0] = '\0';
                    }
                    disk[j] = dirData;
                    strcpy(type[j], "dir");
                    // k - inode
                    inode->ino = k;
                    strcpy(inode->name, name);
                    inode->type = 0;
                    inode->size = 0;
                    inode->blocks = 1;
                    inode->pno = ((struct inode *) disk[cur_dir->entry[0].ino])->direct[0];
                    inode->direct[0] = j;
                    for (int l = 1; l < DIRECT_INODE_NUM; ++l) {
                        inode->direct[l] = -1;
                    }
                    inode->indirect1 = -1;
                    inode->indirect2 = -1;
                    inode->indirect3 = -1;
                    disk[k] = inode;
                    strcpy(type[k], "inode");
                    // add dentry to cur_dir
                    cur_dir->entry[i].ino = k;
                    strcpy(cur_dir->entry[i].name, name);
                    return;
                }
            }
            printf("disk is full!\n");
            return;
        }
    }
    printf("disk is full!\n");
}

void my_rmdir(char *name) {
    for (int i = 2; i < BLOCK_SIZE / DENTRY_SIZE && cur_dir->entry[i].ino >= 0; ++i) {
        if (strcmp(name, cur_dir->entry[i].name) == 0) {
            struct inode *inode = disk[cur_dir->entry[i].ino];
            if (inode->type == 1) {
                printf("%s is a file!\n", name);
                return;
            }
            //struct dir_data *dirData = disk[inode->direct[0]];
            strcpy(type[inode->direct[0]], "empty");
            strcpy(type[inode->ino], "empty");
            //struct dir_data *dirData = disk[inode->ino];
            disk[inode->ino] = NULL;
            disk[inode->direct[0]] = NULL;
            free(inode);
            int j = i;
            for (; j + 1 < BLOCK_SIZE / DENTRY_SIZE && cur_dir->entry[j + 1].ino >= 0; ++j) {
                cur_dir->entry[j].ino = cur_dir->entry[j + 1].ino;
                strcpy(cur_dir->entry[j].name, cur_dir->entry[j + 1].name);
            }
            cur_dir->entry[j].ino = -1;
            return;
        }
    }
    printf("%s is not exist!\n", name);
}

void my_create(char *name) {
    int i = 2;
    for (; i < BLOCK_SIZE / DENTRY_SIZE && cur_dir->entry[i].ino >= 0; ++i) {
        if (strcmp(name, cur_dir->entry[i].name) == 0) {
            printf("%s is already exist!\n", name);
            return;
        }
    }
    if (i == BLOCK_SIZE / DENTRY_SIZE) {
        printf("current directory is full!\n");
        return;
    }
    for (int j = 2; j < BLOCK_NUM; ++j) {
        if (strcmp(type[j], "empty") == 0) {
            for (int k = j + 1; k < BLOCK_NUM; ++k) {
                if (strcmp(type[k], "empty") == 0) {
                    struct file_data *fileData = malloc(BLOCK_SIZE);
                    struct inode *inode = malloc(BLOCK_SIZE);
                    // j - file_data
                    memset(fileData->content, '\0', sizeof(char));
                    disk[j] = fileData;
                    strcpy(type[j], "file");
                    // k - inode
                    inode->ino = k;
                    strcpy(inode->name, name);
                    inode->type = 1;
                    inode->size = 0;
                    inode->blocks = 1;
                    inode->pno = ((struct inode *) disk[cur_dir->entry[0].ino])->direct[0];
                    inode->direct[0] = j;
                    for (int l = 1; l < DIRECT_INODE_NUM; ++l) {
                        inode->direct[l] = -1;
                    }
                    inode->indirect1 = -1;
                    inode->indirect2 = -1;
                    inode->indirect3 = -1;
                    disk[k] = inode;
                    strcpy(type[k], "inode");
                    // add dentry to cur_dir
                    cur_dir->entry[i].ino = k;
                    strcpy(cur_dir->entry[i].name, name);
                    return;
                }
            }
            printf("disk is full!\n");
            return;
        }
    }
    printf("disk is full!\n");
}

void my_rm(char *name) {
    for (int i = 2; i < BLOCK_SIZE / DENTRY_SIZE && cur_dir->entry[i].ino >= 0; ++i) {
        if (strcmp(name, cur_dir->entry[i].name) == 0) {
            struct inode *inode = disk[cur_dir->entry[i].ino];
            if (inode->type == 0) {
                printf("%s is a directory!\n", name);
                return;
            }
            for (int j = 0; j < DIRECT_INODE_NUM; ++j) {
                if (inode->direct[j] > 1) {
                    strcpy(type[inode->direct[j]], "empty");
                }
                disk[inode->direct[j]] = NULL;
            }
            if (inode->indirect1 > 1) {
                strcpy(type[inode->indirect1], "empty");
            }
            disk[inode->indirect1] = NULL;
            if (inode->indirect2 > 1) {
                strcpy(type[inode->indirect2], "empty");
            }
            disk[inode->indirect2] = NULL;
            if (inode->indirect3 > 1) {
                strcpy(type[inode->indirect3], "empty");
            }
            disk[inode->indirect3] = NULL;

            strcpy(type[inode->ino], "empty");
            disk[inode->ino] = NULL;
            free(inode);
            int j = i;
            for (; j + 1 < BLOCK_SIZE / DENTRY_SIZE && cur_dir->entry[j + 1].ino >= 0; ++j) {
                cur_dir->entry[j].ino = cur_dir->entry[j + 1].ino;
                strcpy(cur_dir->entry[j].name, cur_dir->entry[j + 1].name);
            }
            cur_dir->entry[j].ino = -1;
            return;
        }
    }
    printf("%s is not exist!\n", name);
}

void show_open() {
    printf("===== open file list =====\n");
    for (int i = 0; i < MAX_OPEN && i < open_num; ++i) {
        printf("%s/%s (%d)\n", useropen[i].pwd, useropen[i].name, useropen[i].fd);
    }
}

void my_open(char *name) {
    if (open_num == MAX_OPEN) {
        printf("can not open more file!\n");
        return;
    }
    for (int i = 2; i < BLOCK_SIZE / DENTRY_SIZE && cur_dir->entry[i].ino >= 0; ++i) {
        if (strcmp(name, cur_dir->entry[i].name) == 0) {
            int ino = cur_dir->entry[i].ino;
            //printf("%s %d ", cur_dir->entry[i].name,  ino);
            struct inode *inode = disk[ino];
            if (inode->type == 1) {
                useropen[open_num].fd = ino;
                useropen[open_num].offset = 0;
                strcpy(useropen[open_num].name, name);
                strcpy(useropen[open_num].pwd, pwd);
                open_num++;
            } else {
                printf("%s is a file!\n", name);
            }
            show_open();
            return;
        }
    }
    printf("%s is not exist!\n", name);
}

void my_close(int fd) {
    for (int i = 0; i < open_num; ++i) {
        if (useropen[i].fd == fd) {
            int j = i;
            for (; j + 1 < open_num; ++j) {
                useropen[j] = useropen[j + 1];
            }
            open_num--;
            show_open();
            return;
        }
    }
    printf("can not find (%d)!\n", fd);
}

void get_pwd() {
    struct dentry dentry[MAX_DEPTH];
    int i = 0;
    struct dir_data *p = cur_dir;
    while (p->entry[0].ino != 0) {
        dentry[i++] = p->entry[0];
        struct inode *inode = disk[p->entry[1].ino];
        p = (struct dir_data *) disk[inode->direct[0]];
    }
    dentry[i] = p->entry[0];
    pwd[0] = '\0';
    for (int j = i; j >= 0; --j) {
        strcat(pwd, dentry[j].name);
        if (j > 0) strcat(pwd, "/");
    }
    //printf("%s", pwd);
}

int find_block(int x, struct inode *inode) {
    if (x < DIRECT_INODE_NUM) {
        return inode->direct[x];
    } else if (x < DIRECT_INODE_NUM + INDEX_NUM) {
        if (inode->indirect1 < 0) return -10;
        struct index_data *indexData = disk[inode->indirect1];
        return indexData->bno[x - DIRECT_INODE_NUM];
    } else if (x < DIRECT_INODE_NUM + INDEX_NUM + INDEX_NUM * INDEX_NUM) {
        if (inode->indirect2 < 0) return -11;
        struct index_data *indexData = disk[inode->indirect2];
        return indexData->bno[x - DIRECT_INODE_NUM - INDEX_NUM];
    } else {
        if (inode->indirect3 < 0) return -12;
        struct index_data *indexData = disk[inode->indirect3];
        return indexData->bno[x - DIRECT_INODE_NUM - INDEX_NUM - INDEX_NUM * INDEX_NUM];
    }
}

void my_read(int fd) {
    for (int i = 0; i < open_num; ++i) {
        if (useropen[i].fd == fd) {
            struct inode *inode = disk[fd];
            int begin, end;
            printf("name: %s, size: %d, block: %d \n", inode->name, inode->size, inode->blocks);
            printf("enter start and length: ");
            int start, length;
            scanf("%d%d", &start, &length);
            if (start < 0 || length <= 0 || start + length > inode->size) {
                printf("out of range!\n");
            } else {
                int offset = start % BLOCK_SIZE;
                int x = start / BLOCK_SIZE;
                int block = find_block(x++, inode);
                struct file_data *fileData = disk[block];
                int k = 0;
                memset(buffer, '\0', strlen(buffer));
                for (int j = 0; j < length; ++j) {
                    buffer[k++] = fileData->content[offset++];
                    if (offset == BLOCK_SIZE) {
                        block = find_block(x++, inode);
                        if (block <= 0) break;
                        fileData = disk[block];
                        offset = 0;
                    }
                    if (k == BLOCK_SIZE) {
                        printf("%s", buffer);
                        memset(buffer, '\0', BLOCK_SIZE);
                        k = 0;
                    }
                }
                printf("%s\n", buffer);
                memset(buffer, '\0', sizeof(char));
            }
            return;
        }
    }
    printf("can not find (%d)!\n", fd);
}

void my_write(int fd) {
    for (int i = 0; i < open_num; ++i) {
        if (useropen[i].fd == fd) {
            struct inode *inode = disk[fd];
            printf("name: %s, size: %d, block: %d \n", inode->name, inode->size, inode->blocks);
            printf("enter write start: ");
            int start;
            scanf("%d", &start);
            setbuf(stdin, NULL);
            if (start < 0 || start > inode->size) printf("out of range!\n");
            memset(buffer, '\0', BLOCK_SIZE);
            int offset = start % BLOCK_SIZE;
            int x = start / BLOCK_SIZE;
            int block = find_block(x++, inode);
            struct file_data *fileData = disk[block];
            printf("enter your text: ");
            int j = 0;
            char ch;
            while (scanf("%c", &ch) != EOF) {
                if (ch == '$') {
                    // read '\n'
                    scanf("%c", &ch);
                    break;
                }
                // buffer full , write to disk
                if (j >= BLOCK_SIZE) {
                    for (int k = 0; k < BLOCK_SIZE; ++k) {
                        // disk block full , next block
                        if (offset >= BLOCK_SIZE) {
                            block = find_block(x++, inode);
                            // need request new block
                            if (block < 0) {
                                // init new block
                                for (int m = 2; m < BLOCK_NUM; ++m) {
                                    if (strcmp(type[m], "empty") == 0) {
                                        struct file_data *newfile = malloc(BLOCK_SIZE);
                                        disk[m] = newfile;
                                        strcpy(type[m], "file");
                                        if (-1 * block < DIRECT_INODE_NUM) {
                                            for (int l = 0; l < DIRECT_INODE_NUM; ++l) {
                                                if (inode->direct[l] < 0) {
                                                    inode->direct[l] = m;
                                                    break;
                                                }
                                            }
                                        } else if (block == -10) {
                                            inode->indirect1 = m;
                                        } else if (block == -11) {
                                            inode->indirect2 = m;
                                        } else if (block == -12) {
                                            inode->indirect3 = m;
                                        }
                                        block = m;
                                        inode->blocks++;
                                        break;
                                    }
                                }
                            }
                            fileData = disk[block];
                            offset = 0;
                        }
                        fileData->content[offset++] = buffer[k];
                        inode->size++;
                    }
                    j = 0;
                }
                buffer[j++] = ch;
            }
            // write buffer to disk
            for (int k = 0; buffer[k] != '\0'; ++k) {
                // disk full , next disk
                if (offset >= BLOCK_SIZE) {
                    block = find_block(x++, inode);
                    if (block < 0) {
                        // init new block
                        for (int m = 2; m < BLOCK_NUM; ++m) {
                            if (strcmp(type[m], "empty") == 0) {
                                struct file_data *newfile = malloc(BLOCK_SIZE);
                                disk[m] = newfile;
                                strcpy(type[m], "file");
                                if (-1 * block < DIRECT_INODE_NUM) {
                                    for (int l = 0; l < DIRECT_INODE_NUM; ++l) {
                                        if (inode->direct[l] < 0) {
                                            inode->direct[l] = m;
                                            break;
                                        }
                                    }
                                } else if (block == -10) {
                                    inode->indirect1 = m;
                                } else if (block == -11) {
                                    inode->indirect2 = m;
                                } else if (block == -12) {
                                    inode->indirect3 = m;
                                }
                                block = m;
                                inode->blocks++;
                                break;
                            }
                        }
                    }
                    fileData = disk[block];
                    offset = 0;
                }
                fileData->content[offset++] = buffer[k];
                inode->size++;
            }
            return;
        }
    }
    printf("can not find (%d)!\n", fd);
}

void print_type() {
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            int k = i * 64 + j;
            if (strcmp(type[k], "empty") == 0) printf("0");
            else printf("1");
        }
        printf("\n");
    }
}

int main() {
    my_format();

    while (1) {
        get_pwd();
        printf("%s# ", pwd);
        setbuf(stdin, NULL);
        fgets(command, BLOCK_SIZE, stdin);
        command[strlen(command) - 1] = '\0';
        if (strncmp(command, "exit", 4) == 0) break;
        else if (strncmp(command, "ls", 2) == 0) my_ls();
        else if (strncmp(command, "print", 5) == 0) print_type();
        else if (strncmp(command, "cd", 2) == 0) my_cd(command + 3);
        else if (strncmp(command, "mkdir", 5) == 0) my_mkdir(command + 6);
        else if (strncmp(command, "rmdir", 5) == 0) my_rmdir(command + 6);
        else if (strncmp(command, "create", 6) == 0) my_create(command + 7);
        else if (strncmp(command, "rm", 2) == 0) my_rm(command + 3);
        else if (strncmp(command, "open", 4) == 0) my_open(command + 5);
        else if (strncmp(command, "close", 5) == 0) my_close(atoi(command + 6));
        else if (strncmp(command, "show", 4) == 0) show_open();
        else if (strncmp(command, "read", 4) == 0) my_read(atoi(command + 5));
        else if (strncmp(command, "write", 5) == 0) my_write(atoi(command + 6));
        else printf("'%s' command not found!\n", command);
    }

    my_exitsys();
    return 0;
}
