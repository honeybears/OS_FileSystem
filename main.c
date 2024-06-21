#include <stdio.h>
#include "fs.h"
#include "buf.h"
#include "disk.h"
#include <stdlib.h>
#include <string.h>
int main(){
    
   
    CreateFileSystem();
    // for(int i = 0; i< 10; i++){
    //     int a = GetFreeInodeNum();
    //     int b = GetFreeBlockNum();
    //     printf("freeInode : %d ", a);
    //     printf("freeBlock : %d\n", b);
    //     SetBlockBitmap(b);
    //     SetInodeBitmap(a);

    // }
    MakeDir("/b");
    CreateFile("/b/c.txt");
    FileSysInfo* fss = malloc(sizeof(FileSysInfo));
    BufRead(0,fss);
    printf("%d\n",fss->numAllocBlocks);
    free(fss);
    Inode* pInode = (Inode*)malloc(sizeof(Inode));
    GetInode(2,pInode);
    printf("%d\n",pInode->type);
    BufSync();
    DevCloseDisk();
    return 0;
}

