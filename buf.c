#include <stdio.h>
#include <stdlib.h>
#include "buf.h"
#include "queue.h"
#include "disk.h"
#include <string.h>
#include <unistd.h>

struct bufList      bufList[MAX_BUFLIST_NUM];
struct stateList    stateList[MAX_BUF_STATE_NUM];
struct freeList     freeListHead;
struct lruList      lruListHead;

void detachSListById(int blkno,BufState state){
    Buf* tmp;
    CIRCLEQ_FOREACH(tmp,&stateList[state],slist){
        if(tmp->blkno == blkno){
            CIRCLEQ_REMOVE(&stateList[state],tmp,slist);
            return;
        }
    }
}

void detachBListById(int blkno){
    Buf* tmp;
    CIRCLEQ_FOREACH(tmp,&bufList[blkno%MAX_BUFLIST_NUM],blist){
        if(tmp->blkno == blkno){
            CIRCLEQ_REMOVE(&bufList[blkno%MAX_BUFLIST_NUM],tmp,blist);
            return;
        }
    }
}
int getFreeListSize(){
    int size = 0;
    Buf* tmp;
    CIRCLEQ_FOREACH(tmp, &freeListHead, flist){
        size++;
    }
    return size;
}


BOOL isinBufList(Buf* buf){
    Buf* tmp;
    CIRCLEQ_FOREACH(tmp,&bufList[buf->blkno%MAX_BUFLIST_NUM],blist){
        if(tmp->blkno == buf->blkno){
            return TRUE;
        }
    }
    return FALSE;
}

BOOL isinStateList(Buf* buf){
    Buf* tmp;
    CIRCLEQ_FOREACH(tmp,&stateList[buf->state],slist){
        if(tmp->blkno == buf->blkno){
            return TRUE;
        }
    }
    return FALSE;
}
Buf* getFreeBuf(){
    int size = getFreeListSize();
    if(size > 0){
        Buf* freeBuf = CIRCLEQ_FIRST(&freeListHead);
        CIRCLEQ_REMOVE(&freeListHead, freeBuf, flist);
        return freeBuf;
    }
    else{
        
        Buf* victim = CIRCLEQ_FIRST(&lruListHead);
        int hash = victim->blkno%MAX_BUFLIST_NUM;
      

        CIRCLEQ_REMOVE(&lruListHead,victim,llist);
     
           
        if(isinBufList(victim) == TRUE){
        
            detachBListById(victim->blkno);
      
      
        }
       
    
        if(isinStateList(victim) == TRUE){

            if(victim->state == BUF_STATE_DIRTY){
                BufSyncBlock(victim->blkno);
            }
            
            detachSListById(victim->blkno, victim->state);
            
            
        }
        victim -> blkno = BLKNO_INVALID;
        return victim;
    }
}

int GetDirtyListSize(){
    int size = 0;
    Buf* tmp;
      
    CIRCLEQ_FOREACH(tmp,&stateList[BUF_DIRTY_LIST],slist){
        size++;
    }
    return size;
}
int GetLruListSize(){
    int size = 0;
    Buf* tmp;
    CIRCLEQ_FOREACH(tmp,&lruListHead, llist){
        // printf("%d\n",tmp->blkno);
        // sleep(1);
        size++;
    }
    // printf("\n");
 
    return size;
}

void InsertLruList(Buf* buf){
    int size = GetLruListSize();
    
    if(size < MAX_BUF_NUM){
      
        CIRCLEQ_INSERT_TAIL(&lruListHead, buf,llist);     
        return;
        
    }
    else {
      
        // Buf* victim = CIRCLEQ_FIRST(&lruListHead);
        // int hash = victim->blkno%MAX_BUFLIST_NUM;
      

        // CIRCLEQ_REMOVE(&lruListHead,victim,llist);
     
        // CIRCLEQ_INSERT_TAIL(&lruListHead, buf,llist);
     
        // // TODO()        
        // if(isinBufList(victim) == TRUE){
        
        //     detachBListById(victim->blkno);
      
        //     CIRCLEQ_INSERT_TAIL(&bufList[hash], victim,blist);
      
        // }
       
    
        // if(isinStateList(victim) == TRUE){

        //     if(victim->state == BUF_STATE_DIRTY){
        //         BufSyncBlock(victim->blkno);
        //     }
        //     else{
        //         detachSListById(victim->blkno, victim->state);
        //         CIRCLEQ_INSERT_TAIL(&stateList[victim->state], victim,slist);
        //     }
            
        // }
        
       
    }
}

void BufInit(void)
{
    for(int i = 0; i < MAX_BUFLIST_NUM; i++){
        CIRCLEQ_INIT(&bufList[i]);
    }
    for(int i = 0; i < MAX_BUF_STATE_NUM; i++){
        CIRCLEQ_INIT(&stateList[i]);
    }
    
    CIRCLEQ_INIT(&freeListHead);
    CIRCLEQ_INIT(&lruListHead);
    for(int i = 0; i < MAX_BUF_NUM; i++){
        Buf* block = (Buf*)malloc(sizeof(Buf));
        block->blkno = BLKNO_INVALID;
        block->state = BUF_STATE_CLEAN;
        block->pMem = malloc(BLOCK_SIZE);
        CIRCLEQ_INSERT_TAIL(&freeListHead, block,flist);
    }
}

Buf* BufFind(int blkno)
{
    Buf* tmp;
    int hash = blkno % MAX_BUFLIST_NUM;
    CIRCLEQ_FOREACH(tmp, &bufList[hash],  blist){
        if(tmp-> blkno == blkno){
            return tmp;
        }
    }
    return NULL;
}


void BufRead(int blkno, char* pData)
{
    Buf* tmp;
    int hash = blkno % MAX_BUFLIST_NUM;

    CIRCLEQ_FOREACH(tmp, &bufList[hash],  blist){
        
        if(tmp-> blkno == blkno){
            memcpy(pData,(tmp->pMem),BLOCK_SIZE);
            InsertLruList(tmp); 
            return;
           
        }
    }

   

    // TODO :free list link 만들어지면 malloc 대체
    Buf* newBuf = getFreeBuf();
    
    newBuf->blkno = blkno;
    newBuf->state = BUF_STATE_CLEAN;
    DevReadBlock(blkno,newBuf->pMem);
    
    
    memcpy(pData,newBuf->pMem,BLOCK_SIZE);
    
    CIRCLEQ_INSERT_HEAD(&bufList[hash],newBuf,blist);

    CIRCLEQ_INSERT_TAIL(&stateList[BUF_CLEAN_LIST], newBuf, slist);

    InsertLruList(newBuf);
}


void BufWrite(int blkno, char* pData)
{
    Buf* tmp;
    int hash = blkno % MAX_BUFLIST_NUM;
    

    CIRCLEQ_FOREACH(tmp, &bufList[hash],  blist){
        if(tmp-> blkno == blkno){
            memcpy((tmp->pMem),pData,BLOCK_SIZE);
           
            
            if(tmp->state == BUF_STATE_CLEAN){

                CIRCLEQ_REMOVE(&stateList[BUF_CLEAN_LIST], tmp, slist);

                tmp->state = BUF_STATE_DIRTY;
                
                CIRCLEQ_INSERT_TAIL(&stateList[BUF_DIRTY_LIST], tmp, slist);

            }
            InsertLruList(tmp);
            return;
        }
    }

    // TODO :free list link 만들어지면 malloc 대체
    Buf* newBuf = getFreeBuf();
    newBuf->blkno = blkno;
    newBuf->state = BUF_STATE_DIRTY;

    memcpy(newBuf->pMem,pData,BLOCK_SIZE);
    
    CIRCLEQ_INSERT_HEAD(&bufList[hash], newBuf, blist);

    CIRCLEQ_INSERT_TAIL(&stateList[BUF_DIRTY_LIST], newBuf, slist);

    InsertLruList(newBuf);

}

void BufSync(void)
{
    Buf* tmp;
    // CIRCLEQ_FOREACH(tmp,&stateList[BUF_DIRTY_LIST],slist){
    //     CIRCLEQ_REMOVE(&stateList[BUF_DIRTY_LIST], tmp, slist);
    //     DevWriteBlock(tmp->blkno, tmp->pMem);
    //     tmp->state = BUF_STATE_CLEAN;
       
    //     CIRCLEQ_INSERT_TAIL(&stateList[BUF_CLEAN_LIST], tmp, slist);
    // }
    tmp = CIRCLEQ_FIRST(&stateList[BUF_DIRTY_LIST]);
    int size = GetDirtyListSize();
    for(int i = 0; i < size; i++){
       
        Buf* var = CIRCLEQ_NEXT(tmp,slist);

        CIRCLEQ_REMOVE(&stateList[BUF_DIRTY_LIST], tmp, slist);
        DevWriteBlock(tmp->blkno, tmp->pMem);
        tmp->state = BUF_STATE_CLEAN;
       
        CIRCLEQ_INSERT_TAIL(&stateList[BUF_CLEAN_LIST], tmp, slist);
        tmp = var;
    }
   
    
}

void BufSyncBlock(int blkno)
{
    Buf* tmp;
    CIRCLEQ_FOREACH(tmp, &stateList[BUF_DIRTY_LIST], slist){

        if(tmp->blkno == blkno){

            CIRCLEQ_REMOVE(&stateList[BUF_DIRTY_LIST], tmp, slist);

            DevWriteBlock(tmp->blkno, tmp->pMem);

            tmp->state = BUF_STATE_CLEAN;

            CIRCLEQ_INSERT_TAIL(&stateList[BUF_CLEAN_LIST], tmp, slist);

            return;
        }
    }
}


int GetBufInfoInStateList(BufStateList listnum, Buf* ppBufInfo[], int numBuf)
{
    Buf* tmp;

    int bufferContainedCounter = 0;

    CIRCLEQ_FOREACH(tmp, &stateList[listnum], slist){
        if(numBuf > bufferContainedCounter){
            ppBufInfo[bufferContainedCounter++] = tmp;
        }
    }
    return bufferContainedCounter;
}

int GetBufInfoInLruList(Buf* ppBufInfo[], int numBuf)
{
    Buf* tmp;

    int bufferContainedCounter = 0;

    CIRCLEQ_FOREACH(tmp, &lruListHead, llist){
        if(numBuf > bufferContainedCounter){
            ppBufInfo[bufferContainedCounter++] = tmp;
        }
    }
    return bufferContainedCounter;
}



int GetBufInfoInBufferList(int index, Buf* ppBufInfo[], int numBuf)
{
    Buf* tmp;

    int bufferContainedCounter = 0;

    CIRCLEQ_FOREACH(tmp, &bufList[index], blist){
        if(numBuf > bufferContainedCounter){
            ppBufInfo[bufferContainedCounter++] = tmp;
        }
    }
    return bufferContainedCounter;
}

