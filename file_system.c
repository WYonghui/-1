#include"file_system.h"
#include<stdlib.h>
#include<string.h>

char path[64] = {"fil_sys@fil_sys:~"};

/*
*函数功能：初始化模拟文件系统
*入口参数：
*         Disk，磁盘文件
*/
void init_fs(FILE* Disk)
{
    fread(&filesys, sizeof(Filesys), 1, Disk);//读取管理块内容
    
    //printf("filesys.inode_map[0] = %d\n", filesys.inode_map[0]);

    inode_num = 0;  //根目录i结点编号为0
    p_dir = 0;
    open_dir(Disk, inode_num);  //打开根目录
    if((!filesys.inode_map[0]) && (!filesys.ninode))
    {//模拟文件系统初次使用，自动执行格式化操作
        format_fs(Disk);
        //printf("format_fs is called!\n");
    }
    //printf("init_fs:cur_inode.block_num = %d\n", cur_inode.block_num);
}

void close_fs(FILE* Disk)
{
    fseek(Disk, 0, SEEK_SET);
    fwrite(&filesys, sizeof(Filesys), 1, Disk); //写入管理块
    
    //printf("close_fs:inode_num = %d\n", inode_num);
    close_dir(Disk, inode_num); //关闭当前目录
}

//打开当前目录
int open_dir(FILE* Disk, int inode_num)
{
    int i = 0;
    int cur_block;  //当前读取的磁盘块号
    //Dir *p_dir;

    fseek(Disk, INODE_ADDR+inode_num*(sizeof(Inode)), SEEK_SET);
    fread(&cur_inode, sizeof(Inode), 1, Disk);
    cur_block = cur_inode.i_block; //第一个磁盘块编号
    dir_num = cur_inode.file_size/sizeof(Dir); //目录项数

	p_dir = (Dir*)malloc(sizeof(Dir)*dir_num);

    //while(filesys.block_map[cur_block] != END_OF_FILE)
    while(i<cur_inode.block_num-1)
    { //将当前目录文件读入内存
        fseek(Disk, BIOCK_ADDR+sizeof(BLOCKSIZE)*cur_block, SEEK_SET);
        fread(p_dir+i*sizeof(BLOCKSIZE), sizeof(Dir), DIR_PER_BLOCK, Disk);
        cur_block = filesys.block_map[cur_block];
        i++;
    }

    fseek(Disk, BIOCK_ADDR+sizeof(BLOCKSIZE)*cur_block, SEEK_SET);
    fread(p_dir+i*sizeof(BLOCKSIZE), sizeof(Dir), dir_num-(cur_inode.block_num-1)*DIR_PER_BLOCK, Disk);

    return 1;
}

//关闭当前目录
int close_dir(FILE* Disk, int inode_num)
{
    int cur_block;
    int i = 0;
    cur_block = cur_inode.i_block; //第一个磁盘块编号
    //printf("cur_inode.block_num = %d\n", cur_inode.block_num);
    //目录项信息写回磁盘块
    //while(filesys.block_map[cur_block]!=END_OF_FILE)
    while(i<cur_inode.block_num-1)
    {
        fseek(Disk, BIOCK_ADDR+sizeof(BLOCKSIZE)*cur_block, SEEK_SET);
        fwrite(p_dir+i*sizeof(BLOCKSIZE), sizeof(Dir), DIR_PER_BLOCK, Disk);
        cur_block = filesys.block_map[cur_block];
        i++;
    }

    fseek(Disk, BIOCK_ADDR+sizeof(BLOCKSIZE)*cur_block, SEEK_SET);
    fwrite(p_dir+i*sizeof(BLOCKSIZE), sizeof(Dir), dir_num-(cur_inode.block_num-1)*DIR_PER_BLOCK, Disk);

    if(p_dir) {free(p_dir); p_dir=0;}
    //i结点信息写回磁盘块
    fseek(Disk, INODE_ADDR+inode_num*(sizeof(Inode)), SEEK_SET);
    fwrite(&cur_inode, sizeof(Inode), 1, Disk);

    //printf("close_dir:cur_inode.block_num = %d\n", cur_inode.block_num);
    return 1;
}

//文件系统格式化
int format_fs(FILE* Disk)
{
    if(filesys.ninode == MAXINODES - 1) return 1; //重复格式化文件系统，可直接跳过格式化操作
    //清除i结点位视图内容
    memset(filesys.inode_map, 0, MAXINODES*sizeof(int));
    filesys.inode_map[0]=1; //保留根目录i结点
    filesys.ninode = MAXINODES - 1;

    //清除磁盘块位视图内容
    memset(filesys.block_map, 0, sizeof(filesys.block_map));
    filesys.block_map[0] = END_OF_FILE; //保留根目录磁盘块
    filesys.nblock = MAXBOLCKS - 1;

    inode_num = 0;

    //fseek(Disk, INODE_ADDR, SEEK_SET);
    //fread(&cur_inode, sizeof(Inode), 1, Disk);

    cur_inode.i_type = TYPE_DIR;
    cur_inode.i_block = 0;
    cur_inode.block_num = 1;
    cur_inode.file_size = 2*sizeof(Dir);
    fseek(Disk, INODE_ADDR, SEEK_SET);  //根目录i结点始终在磁盘中
    fwrite(&cur_inode, sizeof(Inode), 1, Disk); 

    dir_num = 2;

    if(p_dir) {free(p_dir); p_dir = 0;}
    p_dir = (Dir*)malloc(2*sizeof(Dir));

    strcpy(p_dir->name, ".");
    p_dir->inode_num = 0;
    strcpy((p_dir+sizeof(Dir))->name, "..");
    (p_dir+sizeof(Dir))->inode_num = 0;

    //printf("format_fs:cur_inode.block_num = %d\n", cur_inode.block_num);

    return 1;
}

//显示目录
void show(FILE* Disk, int inode_num)
{
    int i;
    Dir* local_dir = p_dir;

    printf("show:dir_num = %d\n", dir_num);
    printf("show:p_dir->name = %s\n",(p_dir+(dir_num-1)*sizeof(Dir))->name);
    printf("show:p_dir->inode_num = %d\n",(p_dir+(dir_num-1)*sizeof(Dir))->inode_num);
    for(i=0; i<dir_num; i++)
    {
        if(checktype(Disk, (int)(local_dir->inode_num)) == TYPE_DIR)
        {    printf("\e[36m%-20s\033[0m", local_dir->name); printf("%d show:1\n", local_dir->inode_num );}
        else {printf("%-20s", local_dir->name); printf("%d show:1\n", local_dir->inode_num );}
        local_dir += sizeof(Dir);
        if(!((i+1)%4)) printf("\n");
    }
    
    printf("\n");
}

int checktype(FILE* Disk, int inode_num)
{
    Inode l_inode;
    
    fseek(Disk, INODE_ADDR+inode_num*(sizeof(Inode)), SEEK_SET);
    fread(&l_inode, sizeof(Inode), 1, Disk);
    return l_inode.i_type;
}

//新建目录项
int make_file(FILE* Disk, char* name, int type)
{
    int new_inode, new_block;
    Dir * local_dir;
    int i, cur_block = cur_inode.i_block;

    printf("make_file:name = %s\n", name);
    if(filesys.ninode == 0 || filesys.nblock==0) {printf("创建%s失败！\n", name); return -1;}
    else if(checkname(name)) {printf("创建%s失败！\n", name); return -1;}
    
    if(dir_num/DIR_PER_BLOCK != (dir_num+1)/DIR_PER_BLOCK) //本目录需要增加磁盘块
    {   printf("add block!\n");
        if(filesys.nblock==1) {printf("创建%s失败！\n", name); return -1;}
        new_block = apply_block();
        for(i=1; i<cur_inode.block_num; i++)
        { //找到本目录最后一块磁盘块号
            cur_block = filesys.block_map[cur_block];
        }
        filesys.block_map[cur_block] = new_block; //新磁盘块添加到最后一个磁盘块后面
        filesys.block_map[new_block] = END_OF_FILE;
        cur_inode.block_num++;
    }
    cur_inode.file_size += sizeof(Dir); //当前目录文件大小增加

    local_dir = (Dir*)malloc((++dir_num)*sizeof(Dir));
    memmove(local_dir, p_dir, (dir_num-1)*sizeof(Dir));
    free(p_dir);
    p_dir = local_dir; printf("make_file:dir_num = %d\n",dir_num);
    local_dir += (dir_num-1)*sizeof(Dir);

    new_inode = apply_inode(); printf("make_file:new_inode = %d\n", new_inode);
    strcpy(local_dir->name, name); printf("local_dir->name = %s\n",local_dir->name);
    local_dir->inode_num = (short)new_inode;
    printf("p_dir->name = %s\n",(p_dir+(dir_num-1)*sizeof(Dir))->name);
    printf("p_dir->inode_num = %d\n",(p_dir+(dir_num-1)*sizeof(Dir))->inode_num);

    show(Disk, inode_num);
    if(type == TYPE_DIR) init_dir_inode(Disk, new_inode);
    else if(type == TYPE_FILE) {init_file_inode(Disk, new_inode); printf("type:FILE\n");}
    show(Disk, inode_num);
    return 0;
}

//初始化新建目录
void init_dir_inode(FILE* Disk, int new_inode)
{
    Inode local_inode;
    int new_block;
    Dir new_dir[2];

    local_inode.i_type = TYPE_DIR;
    new_block = apply_block();
    local_inode.i_block = new_block;
    local_inode.block_num = 1;
    local_inode.file_size = 2*sizeof(Dir);
    //新建文件i结点写入磁盘
    fseek(Disk, INODE_ADDR+new_inode*(sizeof(Inode)), SEEK_SET);
    fwrite(&local_inode, sizeof(Inode), 1, Disk);

    strcpy(new_dir[0].name, ".");
    new_dir[0].inode_num = (short)new_inode;
    strcpy(new_dir[1].name, "..");
    new_dir[0].inode_num = (short)inode_num;
    //新建目录i结点信息写回磁盘块
    fseek(Disk, BIOCK_ADDR+new_block*(sizeof(BLOCKSIZE)), SEEK_SET);
    fwrite(&new_dir, sizeof(Dir), 2, Disk);

}

//初始化新建文件
void init_file_inode(FILE* Disk, int new_inode)
{
    Inode local_inode;

    local_inode.i_type = TYPE_FILE;
    local_inode.block_num = 0;
    local_inode.file_size = 0;

    //新建文件i结点写入磁盘
    fseek(Disk, INODE_ADDR+new_inode*(sizeof(Inode)), SEEK_SET);
    fwrite(&local_inode, sizeof(Inode), 1, Disk);
    
}

//检查重名   重名返回1;否则，返回0
int checkname(char *name)
{
    Dir* local_dir = p_dir;
    int i = dir_num;
    printf("check:dir_num = %d\n", dir_num);

    while(i > 0)
    {
        if(!strcmp(local_dir->name, name)) return 1;
        i--; //local_dir++;
        local_dir += sizeof(Dir);
    }
    return 0;
}

//申请新磁盘 返回新磁盘号
int apply_block()
{
    int i = 0;

    if(filesys.nblock <= 0) return -1;
    while(i < MAXBOLCKS)
    {
        if(filesys.block_map[i] == 0)
        {
            filesys.block_map[i] == END_OF_FILE; 
            break;
        }
        else i++;
    }
    
    return i;
}

//申请新i结点 返回新i结点号
int apply_inode()
{
    int i = 0;
    while(i < MAXINODES)
    {
        if(filesys.inode_map[i] == 0) 
        {
            filesys.inode_map[i] == 1;
            break;
        }
        else i++;
    }
    return i;
}


//修改当前路径
void change_path(char *name)
{
    int path_length;
    if(!(strcmp(".", name)))
    { //当前目录
        return ;
    }
    else if(!(strcmp("..", name)))
    { //父级目录
        if(strcmp(path, "fil_sys@fil_sys:~"))
		{ //不是根目录
			path_length = strlen(path);
		    while(path[path_length-1] != '/')
		        path_length--;
            path[path_length-1] = '\0';
        }
        //是根目录，则不改变路径
    }
    else
    {
        strcat(path, "/");
        strcat(path, name);
    }
    return ;
}






