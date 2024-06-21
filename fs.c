#include <stdio.h>
#include <stdlib.h>
#include "buf.h"
#include "fs.h"
#include <string.h>
#include <unistd.h>

FileDesc pFileDesc[MAX_FD_ENTRY_MAX];
FileSysInfo* pFileSysInfo;

BOOL appendDirEntry(DirEntry* dirBlock, int inode_num, char* name);
void FileSysInit(void) {
    DevCreateDisk();
    BufInit();
    for(int i = 0; i < FS_DISK_CAPACITY/BLOCK_SIZE; i++){
        char* initBuf = malloc(BLOCK_SIZE);
        memset(initBuf,0,BLOCK_SIZE);
        BufWrite(i,initBuf);
    }
}


int CreateFile(const char *szFileName) {
    // 파일 경로 분석
    char *filePath = strdup(szFileName);
    char *paths[100];
    for (int i = 0; i < 100; i++) {
        paths[i] = malloc(sizeof(char *));
    }
    int pathcnt = 0;
    char *upperPath = strtok(filePath, "/");
    paths[pathcnt] = upperPath;
    while (upperPath != NULL) {
        pathcnt++;
        upperPath = strtok(NULL, "/");
        paths[pathcnt] = upperPath;
    }

    int curPath = 0;
    int inodeNum = 0;
    Inode *pInode = malloc(sizeof(Inode));
    DirEntry parentDirBlock[NUM_OF_DIRENT_PER_BLOCK];

    // 루트 디렉토리부터 시작하여 파일의 부모 디렉토리 탐색
    while (curPath < pathcnt - 1) {
        GetInode(inodeNum, pInode);
        BOOL found = FALSE;
        for (int i = 0; i < pInode->allocBlocks; i++) {
            BufRead(pInode->dirBlockPtr[i], parentDirBlock);
            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                if (strcmp(parentDirBlock[j].name, paths[curPath]) == 0) {
                    inodeNum = parentDirBlock[j].inodeNum;
                    found = TRUE;
                    break;
                }
            }
            if (found) break;
        }
        
        curPath++;
    }

    // 파일 생성 및 정보 설정
    int newFileInodeNo = GetFreeInodeNum();
    
    SetInodeBitmap(newFileInodeNo);

    pInode->allocBlocks = 0;
    pInode->type = FILE_TYPE_FILE;
    pInode->size = 0;
    PutInode(newFileInodeNo, pInode);

    // 부모 디렉토리에 파일 엔트리 추가
    GetInode(inodeNum, pInode);
    for (int i = 0; i < pInode->allocBlocks; i++) {
        BufRead(pInode->dirBlockPtr[i], parentDirBlock);
        if (appendDirEntry(parentDirBlock, newFileInodeNo, paths[pathcnt - 1]) == TRUE) {
            BufWrite(pInode->dirBlockPtr[i], parentDirBlock);
            break;
        }
    }

    // File descriptor table 및 file object 설정 (OpenFile 함수와 동일)
    int fd = -1;
    for (int i = 0; i < MAX_FD_ENTRY_MAX; i++) {
        if (!pFileDesc[i].bUsed) {
            pFileDesc[i].bUsed = TRUE;
            pFileDesc[i].pOpenFile = malloc(sizeof(File));
            pFileDesc[i].pOpenFile->inodeNum = newFileInodeNo;
            pFileDesc[i].pOpenFile->fileOffset = 0;
            fd = i;
            break;
        }
    }

    // FileSysInfo 업데이트
    BufRead(0, pFileSysInfo);
    pFileSysInfo->numAllocInodes++;
    BufWrite(0, pFileSysInfo);

   
    return fd;
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

int MakeDir(const char *szDirName) {
    // (1) 디렉토리 경로 분석 (CreateFile 함수와 동일)
    char *dirPath = strdup(szDirName);
    char *paths[100];
    for (int i = 0; i < 100; i++) {
        paths[i] = malloc(sizeof(char *));
    }
    int pathcnt = 0;
    char *upperPath = strtok(dirPath, "/");
    paths[pathcnt] = upperPath;
    while (upperPath != NULL) {
        pathcnt++;
        upperPath = strtok(NULL, "/");
        paths[pathcnt] = upperPath;
    }

    int curPath = 0;
    int inodeNum = 0;
    Inode *pInode = malloc(sizeof(Inode));
    DirEntry parentDirBlock[NUM_OF_DIRENT_PER_BLOCK];

    // (2) 루트 디렉토리부터 시작하여 부모 디렉토리 탐색 (CreateFile 함수와 동일)
    while (curPath < pathcnt - 1) {
        GetInode(inodeNum, pInode);
        BOOL found = FALSE;
        for (int i = 0; i < pInode->allocBlocks; i++) {
            BufRead(pInode->dirBlockPtr[i], parentDirBlock);
            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                if (strcmp(parentDirBlock[j].name, paths[curPath]) == 0) {
                    inodeNum = parentDirBlock[j].inodeNum;
                    found = TRUE;
                    break;
                }
            }
            if (found) break;
        }
        curPath++;
    }

    // (3) 새 디렉토리 생성 및 정보 설정
    int newDirInodeNo = GetFreeInodeNum();
    
   
    SetInodeBitmap(newDirInodeNo);

    int newDirBlockNo = GetFreeBlockNum();
    
    SetBlockBitmap(newDirBlockNo);

    DirEntry newDirBlock[NUM_OF_DIRENT_PER_BLOCK];
    memset(newDirBlock, 0, BLOCK_SIZE); // newDirBlock 초기화
    newDirBlock[0].inodeNum = newDirInodeNo;
    strcpy(newDirBlock[0].name, ".");
    newDirBlock[1].inodeNum = inodeNum; // 부모 디렉토리의 inode 번호
    strcpy(newDirBlock[1].name, "..");
    BufWrite(newDirBlockNo, newDirBlock);

    pInode->allocBlocks = 1;
    pInode->dirBlockPtr[0] = newDirBlockNo;
    pInode->type = FILE_TYPE_DIR;
    pInode->size = sizeof(DirEntry) * 2; // "." 및 ".." 엔트리 크기
    PutInode(newDirInodeNo, pInode);

    // (4) 부모 디렉토리에 새 디렉토리 엔트리 추가 (CreateFile 함수와 동일)
    GetInode(inodeNum, pInode);
    for (int i = 0; i < pInode->allocBlocks; i++) {
        BufRead(pInode->dirBlockPtr[i], parentDirBlock);
        if (appendDirEntry(parentDirBlock, newDirInodeNo, paths[pathcnt - 1]) == TRUE) {
            BufWrite(pInode->dirBlockPtr[i], parentDirBlock);
            break;
        }
    }

    // (5) FileSysInfo 업데이트
    BufRead(0, pFileSysInfo);
    pFileSysInfo->numAllocBlocks++;
    pFileSysInfo->numFreeBlocks--;
    pFileSysInfo->numAllocInodes++;
    BufWrite(0, pFileSysInfo);

  
    return 0; // 성공적으로 디렉토리 생성
}

// int     MakeDir(const char* szDirName)
// {   
//     char* dirPath = strdup(szDirName);
//     char* paths[100];
//     for(int i = 0; i < 100; i++){
//         paths[i] = malloc(sizeof(char*));
//     }
//     int pathcnt = 0;
//     char* upperPath = strtok(dirPath,"/");
//     paths[pathcnt] = upperPath;
//     while(upperPath != NULL){
//         pathcnt ++;
//         upperPath = strtok(NULL,"/");
//         paths[pathcnt] = upperPath;
//     }
//     printf("%d",pathcnt);
//     int curpath = 0;
//     int inodeNum = 0;
//     Inode* pInode = malloc(sizeof(Inode));
//     Inode* rootInode = malloc(sizeof(Inode));
//     DirEntry newDirBlock[NUM_OF_DIRENT_PER_BLOCK];
//     for(int i = 0; i < NUM_OF_DIRENT_PER_BLOCK; i++){
//         newDirBlock[i].inodeNum = -1;
//         newDirBlock[i].name[0] = '\0';
//     }
//     DirEntry parentDirBlock[NUM_OF_DIRENT_PER_BLOCK];
//     GetInode(0,rootInode);
//     int newDirInodeNo = GetFreeInodeNum();
//     int newDirBlockNo = GetFreeBlockNum();
//     if(pathcnt == 1){
//         for(int i = 0; i < rootInode->allocBlocks; i++){
//             BufRead(rootInode->dirBlockPtr[i],parentDirBlock);
//             if(appendDirEntry(parentDirBlock,newDirInodeNo,paths[pathcnt-1]) == TRUE){
//                 newDirBlock[0].inodeNum = newDirBlockNo;
//                 memcpy(newDirBlock[0].name,".",1);
//                 newDirBlock[1].inodeNum = 0;
//                 memcpy(newDirBlock[1].name,"..",2);
//                 BufWrite(newDirBlockNo,newDirBlock);
//                 SetBlockBitmap(newDirBlockNo);
//                 pInode->allocBlocks = 1;
//                 pInode->dirBlockPtr[0] = newDirBlockNo;
//                 pInode->type = FILE_TYPE_DIR;
//                 pInode->size = sizeof(DirEntry);
//                 PutInode(newDirInodeNo,pInode);
//                 SetInodeBitmap(newDirInodeNo);
//                 break;
//             }
//         }
//     }
//     // while(curpath != pathcnt){
//     //     pInode = malloc(sizeof(Inode));
//     //     GetInode(inodeNum, pInode);
        
        
//     // }
    
//     // GetInode(0,pInode);
//     // int block_num = GetFreeBlockNum();
//     // int inode_num = GetFreeInodeNum();
//     // for(int i = 0; i < pInode->allocBlocks; i++){
//     //     DirEntry dirBlock[NUM_OF_DIRECT_BLOCK_PTR];
//     //     BufRead(pInode->dirBlockPtr[i],dirBlock);
//     //     for(int i = 0; NUM_OF_DIRECT_BLOCK_PTR; i++){
            
//     //     }


//     // }
    

    
// }


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

    printf("block num : %d\n",block_num);
    printf("inode num : %d\n",inode_num);

    pBuf[0].inodeNum = inode_num;
    memcpy(pBuf[0].name,".",1);
    for(int i = 1; i < NUM_OF_DIRENT_PER_BLOCK; i++){
        pBuf[i].inodeNum = -1;
        pBuf[i].name[0] = '\0';

    }
    BufWrite(block_num, pBuf);
    
    //
    // DirEntry s[NUM_OF_DIRENT_PER_BLOCK];
    // BufRead(block_num, s);
    // for(int i = 0 ;i < NUM_OF_DIRENT_PER_BLOCK; i++){
    //     printf("%d, %d, %s\n", i, s[i].inodeNum,s[i].name);
    // }

    SetBlockBitmap(block_num);

    pFileSysInfo = (FileSysInfo*)malloc(BLOCK_SIZE);
    
    pFileSysInfo->blocks = FS_DISK_CAPACITY / BLOCK_SIZE;
    pFileSysInfo->rootInodeNum = 0;
    pFileSysInfo->diskCapacity = FS_DISK_CAPACITY;
    pFileSysInfo->numAllocBlocks = 0;
    pFileSysInfo->numFreeBlocks = FS_DISK_CAPACITY / BLOCK_SIZE;
    pFileSysInfo->numAllocInodes = 0;
    pFileSysInfo->blockBitmapBlock = BLOCK_BYTEMAP_BLOCK_NUM;
    pFileSysInfo->inodeBitmapBlock = INODE_BYTEMAP_BLOCK_NUM;
    pFileSysInfo->inodeListBlock = INODELIST_BLOCK_FIRST;
    pFileSysInfo->dataRegionBlock = INODELIST_BLOCK_FIRST + INODELIST_BLOCKS;
    
    pFileSysInfo->numAllocBlocks++;
    pFileSysInfo->numFreeBlocks--;
    pFileSysInfo->numAllocInodes++;

    BufWrite(FILESYS_INFO_BLOCK,pFileSysInfo);

    Inode* pInode = malloc(sizeof(Inode));
    pInode->dirBlockPtr[0] = block_num;
    pInode->type = FILE_TYPE_DIR;
    pInode->size = sizeof(DirEntry) * 1;
    pInode->allocBlocks = 1;
    PutInode(inode_num,pInode);

        SetInodeBitmap(inode_num);

}


void    OpenFileSystem(void){

}
void    CloseFileSystem(void){

}


BOOL appendDirEntry(DirEntry* dirBlock, int inode_num, char* name){
    for(int i = 0; i < NUM_OF_DIRENT_PER_BLOCK; i++){
        if(dirBlock[i].inodeNum == -1){
            dirBlock[i].inodeNum = inode_num;
            memcpy(dirBlock[i].name ,name, MAX_NAME_LEN);
            return TRUE;
        }
    }
    return FALSE;
}