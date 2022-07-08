#pragma once
#include <string>
#include "global.h"

class Server {
public:
	void createPipe();
	void forceStop();
	bool runCmd(std::string cmd);
private:
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char ReadBuff[4097];
	DWORD ReadNum;
	HANDLE g_hChildStd_IN_Rd = NULL;
	HANDLE g_hChildStd_IN_Wr = NULL;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;
	SECURITY_ATTRIBUTES sa;
	BOOL bRet;
	BOOL bWet;
	bool canSend = false;
	bool started = false;
};