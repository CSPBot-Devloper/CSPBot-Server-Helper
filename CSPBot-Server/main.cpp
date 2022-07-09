#define BUFSIZE 4096 //读取字节

#include <iostream>
#include <windows.h>
#include <tchar.h>
#include "server.h"
#include <string>
#include <thread>
#include "websocket.h"
#include <boost/bind.hpp>
#include <Nlohmann/json.hpp>
#include "helper.h"
#include <fstream>

#pragma warning(disable:4251);

using json = nlohmann::json;
using namespace std;
bool canRead = true;
int point = 0;
CHAR chBuf[4097];
DWORD dwRead, dwWritten;
HANDLE hStdin, hStdout;
BOOL bSuccess;

void* pClient = nullptr;

//设置UTF-8
void setUTF8() {
	SetConsoleOutputCP(65001);
	CONSOLE_FONT_INFOEX info = { 0 };
	info.cbSize = sizeof(info);
	info.dwFontSize.Y = 16;
	info.FontWeight = FW_NORMAL;
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &info);
}

//获取stdin输入
void threadInput(Server* server) {
	string cmd = "";
	for (;;) {
		bSuccess = ReadFile(hStdin, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) {
			break;
		}
		
		chBuf[dwRead] = '\0';
		cmd = chBuf;
		server->runCmd(cmd);
	}
}

//Websocket收到消息
void on_message(void* pClient, const std::string data, WsOpcode opcode)
{
	if (!Helper::IsJsonData(data)) {
		WebSockServer::Instance().Send(
			pClient,
			"{\"code\":400,\"packet\":\"Error\",\"data\":\"Json cannot be parsed\"}",
			WsOpcode::TEXT
		);
		return;
	}
	json j = json::parse(data);
	//解析字符串
	webJson wj = Helper::transJson(j);
	if (wj.packet == "heart") {
		time_t nowTime = time(NULL);
		string timeString = to_string(nowTime);
		WebSockServer::Instance().Send(
			pClient, 
			"{\"code\":200,\"packet\":\"heart\",\"data\":\""+timeString+"\"}", 
			WsOpcode::TEXT
		);
	}
	
}

//Websocket接入连接
void on_open(void* pClient)
{
	::pClient = pClient;
}

//Websocket关闭
void on_close(void* pClient, std::string msg)
{
	pClient = nullptr;
}

//获取端口
int getPort() {
	json j;
	ifstream fin("config.json");
	fin >> j;
	return j["port"].get<int>();
}

//开启Websocket服务器
void startWebsocketServer() {
	WebSockServer::Instance().Init(getPort(),
		boost::bind(on_open, _1),
		boost::bind(on_close, _1, _2),
		boost::bind(on_message, _1, _2, _3)
	);
	string s = "Websocket Server is running on ws://127.0.0.1:" + to_string(getPort());
	WriteFile(hStdout, s.c_str(), s.length(), &dwWritten, NULL);
	WebSockServer::Instance().StartServer();
}


//定时发送消息
void timeOutSend() {
	while (1) {
		string j = "{\"packet\":\"cmd\",\"data\":\"\"}";
		WebSockServer::Instance().Send(pClient, j, WsOpcode::TEXT);
		Sleep(5 * 1000);
	}
}

int main() {
	setUTF8();
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (
		(hStdout == INVALID_HANDLE_VALUE) ||
		(hStdin == INVALID_HANDLE_VALUE)
		)
		ExitProcess(1);
	
	//创建服务器类
	Server* server = new Server();
	//开启线程
	std::thread thread1(threadInput, server);
	std::thread thread2(startWebsocketServer);
	std::thread thread3(timeOutSend);
	
	//开启服务器
	server->createPipe();
	
	//停止检测输入
	thread1.detach();

	//停止Websocket
	thread3.detach();
	WebSockServer::Instance().StopServer();
	thread2.join();
	return 0;
}