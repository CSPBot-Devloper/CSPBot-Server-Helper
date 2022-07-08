#pragma once
#include <Windows.h>

extern bool canRead;
extern int point;
extern CHAR chBuf[4097];
extern DWORD dwRead, dwWritten;
extern HANDLE hStdin, hStdout;
extern BOOL bSuccess;
extern void* pClient;