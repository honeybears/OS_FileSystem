#include <stdio.h>
#include <stdlib.h>
#include "buf.h"
#include "fs.h"
#include <string.h>
#include <unistd.h>

FileDesc pFileDesc[MAX_FD_ENTRY_MAX];
FileSysInfo* pFileSysInfo;

void FileSysInit(void) {
    DevCreateDisk();
    BufInit();
    for(int i = 0; i < FS_DISK_CAPACITY/BLOCK_SIZE; i++){
        char* initBuf = malloc(BLOCK_SIZE);
        memset(initBuf,'0',BLOCK_SIZE);
        BufWrite(i,initBuf);
    }
}


int     CreateFile(const char* szFileName)
{

}

int     OpenFile(const char* szFileName)
{

}


int     WriteFile(int fileDesc, char* pBuffer, int length)
{

}

int     ReadFile(int fileDesc, char* pBuffer, int length)
{

}


int     CloseFile(int fileDesc)
{

}

int     RemoveFile(const char* szFileName)
{

}


int     MakeDir(const char* szDirName)
{   
    char *dirPath = strdup(szDirName);
    
}


int     RemoveDir(const char* szDirName)
{

}

int   EnumerateDirStatus(const char* szDirName, DirEntryInfo* pDirEntry, int dirEntrys)
{

}


void    CreateFileSystem(void){
    FileSysInit();
    int block_num = GetFreeBlockNum();

    int inode_num = GetFreeInodeNum();
    DirEntry pBuf[NUM_OF_DIRENT_PER_BLOCK];

   

    pBuf[0].inodeNum = inode_num;
    pBuf[0].name[0] = '0';
    
    BufWrite(block_num, pBuf);
    

    SetBlockBitmap(block_num);

    pFileSysInfo = (FileSysInfo*)malloc(BLOCK_SIZE);
    
    pFileSysInfo->blocks = FS_DISK_CAPACITY / BLOCK_SIZE;
    pFileSysInfo->rootInodeNum = 0;
    pFileSysInfo->diskCapacity = FS_DISK_CAPACITY;
    pFileSysInfo->numAllocBlocks = 7;
    pFileSysInfo->numFreeBlocks = FS_DISK_CAPACITY / BLOCK_SIZE - 7;
    pFileSysInfo->numAllocInodes = 0;
    pFileSysInfo->blockBitmapBlock = BLOCK_BYTEMAP_BLOCK_NUM;
    pFileSysInfo->inodeBitmapBlock = INODE_BYTEMAP_BLOCK_NUM;
    pFileSysInfo->inodeListBlock = INODELIST_BLOCK_FIRST;
    pFileSysInfo->dataRegionBlock = INODELIST_BLOCK_FIRST + INODELIST_BLOCKS;
    
    pFileSysInfo->numAllocBlocks++;
    pFileSysInfo->numFreeBlocks--;
    pFileSysInfo->numAllocBlocks++;

    BufWrite(inode_num,pFileSysInfo);

    SetInodeBitmap(inode_num);
    Inode* pInode = malloc(sizeof(Inode));
    GetInode(0,pInode);
    pInode->dirBlockPtr[0] = block_num;
    pInode->type = FILE_TYPE_DIR;
    pInode->size = sizeof(DirEntry);
    pInode->allocBlocks = 1;
    PutInode(0,pInode);

    
}


void    OpenFileSystem(void){

}
void    CloseFileSystem(void){

}
