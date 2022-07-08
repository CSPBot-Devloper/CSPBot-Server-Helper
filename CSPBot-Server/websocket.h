#pragma once
#include <string>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#if defined(_WIN32) || defined(_WIN64)
#  define WEB_SOCKET_DLL_EXPORT __declspec(dllexport)
#else
#  define WEB_SOCKET_DLL_EXPORT
#endif

enum WsOpcode
{
	BINARY = 1,
	TEXT,
	OTHER
};

struct webJson {
	std::string packet;
	std::string data;
};

//定义回调函数
typedef boost::function<void(void*, const std::string, WsOpcode)> OnMessageFun;//接收消息到来的函数
typedef boost::function<void(void*)> OnOpenFun;                      //连接到来
typedef boost::function<void(void*, std::string)> OnCloseFun;        //连接断开的函数

//单例的服务器
class WEB_SOCKET_DLL_EXPORT WebSockServer
{
private:
	WebSockServer();
public:
	static WebSockServer& Instance();
	bool Init(uint16_t uPort, OnOpenFun openFun, OnCloseFun closeFun, OnMessageFun msgFun, void* pOiService = nullptr);
	/*
	如果调用StartServer,应该另开一个线程来调用，因为程序会在里面循环运行，不会返回，直到另一个线程调用StopServer函数
	*/
	bool StartServer();
	void StopServer();
	void StopListening();
	/*
	发送数据的接口
	pClient为客户端标识
	data 为要发送的数据
	*/
	bool Send(void* pClient, const std::string data, WsOpcode opcode = WsOpcode::BINARY);
	void Close(void* pClient);
	void CloseAll();
	//private:
	//	void ThreadProccess();               //线程函数

public:
	OnOpenFun       m_onOpenFun;
	OnCloseFun      m_onCloseFun;
	OnMessageFun    m_onMsgFun;

private:
	//boost::thread * m_threadMain;              //websock 的循环线程
	bool            m_bServerStart;            //服务是否启动的标志
};

