#include"file_system.h"



int main()
{
    FILE*  Disk;
    char name[64];

    char *command[] = {"format", "mkdir", "mk", "rm", "cd", "close", "read", "write", "ls", "quit"};
    //                    0         1       2    3       4       5       6        7       8     9
    char choice[32];
    int i;
    int local_inode;
    if((Disk=fopen("filesystem.txt", "r+"))==NULL)
    { //打开一个文件，模拟文件管理系统
        Disk = fopen("filesystem.txt", "w+");
        //printf("Open file false!\n");
        //return 0;
    }

    init_fs(Disk); //初始化模拟文件系统
    while(1)
    {
        printf("%s$ ", path);
        scanf("%s", choice);
        for(i=0; i<10; i++)
        {
            if(!(strcmp(choice, command[i]))) break;
        }
        if(i==9) break; //退出
        switch(i)
        {
            case 0:
                format_fs(Disk);
                break;
            case 1:
                scanf("%s", name);
                make_file(Disk, name, TYPE_DIR);
                break;
            case 2:
                scanf("%s", name);
                make_file(Disk, name, TYPE_FILE);
                break;
            case 3:
                break;
            case 4:
                scanf("%s", name);
                if((local_inode=checkname(name))==-1)
                {
                    printf("%s: 没有那个文件或目录\n", name);
                    break;
                }
                if((checktype(Disk,local_inode)) != TYPE_DIR)
                {
                    printf("%s: 没有那个文件\n", name);
                    break;
                }
                if(!enter_dir(Disk, name))
                {
                    change_path(name);
                }
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                break;
            case 8:
                show(Disk, inode_num);
                break;
            default:
                printf("%s: 未找到命令\n",choice);
            
        }


    }
    //printf("main:cur_inode.block_num = %d\n", cur_inode.block_num);
    close_fs(Disk); //关闭文件系统
    //printf("2\n");
    fclose(Disk); //关闭文件
    return 0;
}
