#include <stdio.h>
#include <stdlib.h>
#include "buf.h"
#include "fs.h"
#include <string.h>


void SetInodeBitmap(int inodeno)
{
    char* pBuf = malloc(BLOCK_SIZE);
    BufRead(INODE_BYTEMAP_BLOCK_NUM,pBuf);
    ((unsigned char*)pBuf)[inodeno] |= 1;
    

    BufWrite(INODE_BYTEMAP_BLOCK_NUM,pBuf);   

}


void ResetInodeBitmap(int inodeno)
{
    char* pBuf = malloc(BLOCK_SIZE);
    BufRead(INODE_BYTEMAP_BLOCK_NUM,pBuf);
    ((unsigned char*)pBuf)[inodeno] &= 0;
    

    BufWrite(INODE_BYTEMAP_BLOCK_NUM,pBuf);
}


void SetBlockBitmap(int blkno)
{
    char* pBuf = malloc(BLOCK_SIZE);
    BufRead(BLOCK_BYTEMAP_BLOCK_NUM,pBuf);
    ((unsigned char*)pBuf)[blkno] |= 1;
    
    
    BufWrite(BLOCK_BYTEMAP_BLOCK_NUM,pBuf);
}


void ResetBlockBitmap(int blkno)
{
    char* pBuf = malloc(BLOCK_SIZE);
    BufRead(BLOCK_BYTEMAP_BLOCK_NUM,pBuf);
    ((unsigned char*)pBuf)[blkno] &= 0;


    BufWrite(BLOCK_BYTEMAP_BLOCK_NUM,pBuf);
}


void PutInode(int inodeno, Inode *pInode) {
    char pbuf[BLOCK_SIZE];
    int block_num = INODELIST_BLOCK_FIRST + inodeno / NUM_OF_INODE_PER_BLOCK;
    BufRead(block_num, pbuf);
    Inode *inode_ptr = (Inode *)(pbuf + inodeno % NUM_OF_INODE_PER_BLOCK);
    *inode_ptr = *pInode;
    BufWrite(block_num, pbuf);
}

void GetInode(int inodeno, Inode *pInode) {
    char pbuf[BLOCK_SIZE];
    int block_num = INODELIST_BLOCK_FIRST + inodeno / NUM_OF_INODE_PER_BLOCK;
    BufRead(block_num, pbuf);
    Inode *inode_ptr = (Inode *)(pbuf + inodeno % NUM_OF_INODE_PER_BLOCK);
    memcpy(pInode,inode_ptr,sizeof(Inode));
}

int GetFreeInodeNum(void) {
    char *pbuf = malloc(BLOCK_SIZE);
    BufRead(INODE_BYTEMAP_BLOCK_NUM, pbuf);

    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < 8; j++) {  // 각 바이트의 비트를 확인
            if (!(pbuf[i] & (1 << j))) {
                return (i * 8) + j;  // 비트맵 인덱스 계산
            }
        }
    }
    // for (int i = 0; i < BLOCK_SIZE; i++) {
    //     for (int j = 0; j < 8; j++) {
    //         if (!(buffer[i] & (1 << j))) {
    //             return i * 8 + j;
    //         }
    //     }
    // }
    return -1; // No free inode found
}

int GetFreeBlockNum(void) {
    char *pbuf = malloc(BLOCK_SIZE);
    BufRead(BLOCK_BYTEMAP_BLOCK_NUM, pbuf);

    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < 8; j++) {  // 각 바이트의 비트를 확인
            if (!(pbuf[i] & (1 << j))) {
                return (i * 8) + j;  // 비트맵 인덱스 계산
            }
        }
    }
    return -1; // No free block found
}
