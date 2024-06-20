#include <stdio.h>
#include <stdlib.h>
#include "buf.h"
#include "fs.h"

FileDesc pFileDesc[MAX_FD_ENTRY_MAX];
FileSysInfo* pFileSysInfo;

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

}


int     RemoveDir(const char* szDirName)
{

}

int   EnumerateDirStatus(const char* szDirName, DirEntryInfo* pDirEntry, int dirEntrys)
{

}


void    CreateFileSystem()
{

}


void    OpenFileSystem()
{

}