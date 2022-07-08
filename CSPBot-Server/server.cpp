#include "server.h"
#include <Windows.h>
#include <tchar.h>
#include <string>
#include "global.h"
#include "websocket.h"
#include "helper.h"

using namespace std;

void Server::createPipe()
{
	si = { 0 };
	pi = { 0 };
	ReadBuff[4097] = { 0 };
	ReadNum = 0;
	sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = 0;
	BOOL bRet = CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0);
	BOOL bWet = CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &sa, 0);
	HANDLE hTemp = GetStdHandle(STD_OUTPUT_HANDLE);
	SetStdHandle(STD_OUTPUT_HANDLE, g_hChildStd_OUT_Wr);
	GetStartupInfo(&si);//获取本进程当前的STARTUPINFO结构信息
	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdInput = g_hChildStd_IN_Rd;
	si.hStdError = g_hChildStd_OUT_Wr;
	si.hStdOutput = g_hChildStd_OUT_Wr;

	std::string s = "runner.bat";

	wstring widstr = std::wstring(s.begin(), s.end());
	LPWSTR path = (LPWSTR)widstr.c_str();
	bRet = CreateProcess(NULL, path, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi);
	SetStdHandle(STD_OUTPUT_HANDLE, hTemp);
	CloseHandle(g_hChildStd_OUT_Wr);
	started = true;
	while (ReadFile(g_hChildStd_OUT_Rd, ReadBuff, 4096, &ReadNum, NULL))
	{
		canRead = false;
		ReadBuff[ReadNum] = '\0';
		string str = ReadBuff;
		if (str.find("Done") != string::npos || str.find("Server Started") != string::npos) {
			canSend = true;
		}
		bSuccess = WriteFile(hStdout, ReadBuff, ReadNum, &dwWritten, NULL);
		if (!bSuccess)
			break;
		canRead = true;
	}
}

void Server::forceStop() {
	string cmd = "taskkill  /F /T /PID" + std::to_string(pi.dwProcessId);
	system(cmd.c_str());
}

//服务器运行命令
bool Server::runCmd(string cmd) {
	if (!started) {
		return false;
	}
	if (canSend) {
		if (pClient == nullptr) {
			return false;
		}
		cmd = Helper::replace(cmd, "\r", "");
		cmd = Helper::replace(cmd, "\n", "");
		cmd = Helper::replace(cmd, "\"", "\\\"");
		string j = "{\"packet\":\"cmd\",\"data\":\"" + cmd + "\"}";
		return WebSockServer::Instance().Send(pClient, j, WsOpcode::TEXT);
	}
	else {
		return WriteFile(g_hChildStd_IN_Wr, cmd.c_str(), cmd.length(), &ReadNum, NULL);
	}
	
}