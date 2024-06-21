#include <stdio.h>
#include <stdlib.h>
#include "buf.h"
#include "fs.h"
#include <string.h>


void SetInodeBitmap(int inodeno)
{
    char *pBuf = malloc(BLOCK_SIZE);
    BufRead(INODE_BYTEMAP_BLOCK_NUM, pBuf);

    int byteIndex = inodeno / 8;     // inodeno가 속한 바이트의 인덱스
    int bitIndex = inodeno % 8;      // inodeno가 바이트 내에서 몇 번째 비트인지

    ((unsigned char*)pBuf)[byteIndex] |= (1 << bitIndex);  // 해당 비트를 1로 설정
    BufWrite(INODE_BYTEMAP_BLOCK_NUM, pBuf);

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
    char *pBuf = malloc(BLOCK_SIZE);
    BufRead(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    int byteIndex = blkno / 8;     // inodeno가 속한 바이트의 인덱스
    int bitIndex = blkno % 8;      // inodeno가 바이트 내에서 몇 번째 비트인지

    ((unsigned char*)pBuf)[byteIndex] |= (1 << bitIndex);  // 해당 비트를 1로 설정
    BufWrite(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);
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
    //memcpy(inode_ptr,pInode,sizeof(Inode));

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
        for (int j = 0; j < 8; j++) {
            if (!(pbuf[i] & (1 << j))) {
                return (i*8) + j;
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

    
    for (int i = 0; i < BLOCK_SIZE; i++) { // 블록 범위 수정
        if(i == 0){
            if(!(pbuf[i] & (1<<7))){
                return (i*8) + 7;
            }
            else{
                continue;
            }
        }
        for (int j = 0; j < 8; j++) {
            if (!(pbuf[i] & (1 << j))) {
                return (i*8) + j;
            }
        }
    }
    return -1; // No free block found
}
