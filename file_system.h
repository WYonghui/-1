#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H 1

#include<stdio.h>

#define BLOCKSIZE 512 //磁盘块大小512B
#define MAXINODES 1024 //i节点数
#define MAXBOLCKS 1024 //磁盘块数
#define TYPE_FILE  1
#define TYPE_DIR   2
#define END_OF_FILE 0xFFFF

typedef struct{
    int inode_map[MAXINODES];  //i节点位图
    int block_map[MAXBOLCKS];  //空闲磁盘块位图
    int ninode; //剩余空闲i节点数
    int nblock; //剩余空闲块数
}Filesys;   //管理块

typedef struct{ //i结点的信息在对i结点进行操作时修改，读取时不操作
    int i_type;    //文件类型
    int i_block;   //占用的第一块磁盘块号
    int block_num; //占用的磁盘块数目
    int file_size; //文件大小
}Inode;  //i节点

typedef struct{
    char name[112]; //文件名
    short inode_num; //i节点号
}Dir;  //目录项, 共占16字节。每个磁盘快可存储32个目录项

#define INODE_ADDR (sizeof(Filesys)) //i结点起始地址
#define BIOCK_ADDR (INODE_ADDR+sizeof(Inode)*MAXINODES) //磁盘块起始地址
#define DIR_PER_BLOCK (BLOCKSIZE/(sizeof(Dir)))   //每个磁盘块可存储的最大目录项数

Filesys filesys; //管理块
Dir *p_dir; //目录项表
int inode_num;  //当前目录的i结点号
int dir_num;    //当前目录下目录项数
Inode cur_inode; //当前目录的i结点结构

extern char path[64];

void init_fs(FILE* Disk); //初始化模拟文件系统
void close_fs(FILE* Disk); //关闭模拟文件系统


int format_fs(FILE* Disk); //文件系统格式化
int open_dir(FILE* Disk, int inode_num);//打开目录
int close_dir(FILE* Disk, int inode_num);//关闭目录

void show(FILE* Disk, int inode_num);//显示目录

int make_file(FILE* Disk, char* name, int type); //新建目录项
void init_dir_inode(FILE* Disk, int new_inode);  //初始化新建目录
void init_file_inode(FILE* Disk, int new_inode);  //初始化新建文件 

void change_path(char *name); //修改当前路径 

int checktype(FILE* Disk, int inode_num); //检查类型
int checkname(char *name);   //检查重名
int apply_block();    //申请新磁盘 返回新磁盘号
int apply_inode();    //申请新i结点 返回新i结点号


#endif
