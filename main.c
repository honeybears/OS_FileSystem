#include <stdio.h>
#include "fs.h"
#include "buf.h"
#include "disk.h"
#include <stdlib.h>
#include <string.h>
int main(){
    
    CreateFileSystem();
    int freeNode = GetFreeInodeNum();
    printf("%d",freeNode);
    printf("Hello OS World\n");
    char *s = malloc(BLOCK_SIZE);
    printf("%d\n",sizeof(s));
    memcpy(s,"101111001011",BLOCK_SIZE);

    printf("%s\n",s);
    ((char*)s)[1] |= 1;
    printf("%s\n",s);

    char* initBuf = malloc(BLOCK_SIZE);
    memset(initBuf,'0',BLOCK_SIZE);

    printf("%s\n", initBuf);
    BufWrite(0,initBuf);

    char* pbuf = malloc(BLOCK_SIZE);
    BufRead(0,pbuf);
    printf("%s\n",pbuf);
    char z[] = "/tmp/a/b/c.txt";
    char* tok = strtok(z,"/");
    while(tok != NULL){
        printf("%s\n",tok);
        tok = strtok(NULL,"/");
    }
    
    DevCloseDisk();
    return 0;
}

