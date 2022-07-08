
#include "websocket.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <map>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef server::message_ptr message_ptr;
typedef std::map<void*, websocketpp::connection_hdl> ClientMap;

static server g_server;

static ClientMap g_mapClient;
static boost::shared_mutex rwMutext;
typedef boost::shared_lock<boost::shared_mutex> ReadLock;
typedef boost::unique_lock<boost::shared_mutex> WriteLock;

static void on_message(WebSockServer* pWebserver, websocketpp::connection_hdl hdl, message_ptr msg) {
	if (!pWebserver->m_onMsgFun.empty())
	{
		WsOpcode opcode = WsOpcode::OTHER;
		if (msg->get_opcode() == websocketpp::frame::opcode::value::TEXT)
		{
			opcode = WsOpcode::TEXT;
		}
		else if (msg->get_opcode() == websocketpp::frame::opcode::value::BINARY)
		{
			opcode = WsOpcode::BINARY;
		}
		pWebserver->m_onMsgFun(hdl.lock().get(), msg->get_payload(), opcode);
	}
}

static void on_open(WebSockServer* pWebserver, websocketpp::connection_hdl hdl)
{
	WriteLock writeLock(rwMutext);
	g_mapClient.insert(std::pair<void*, websocketpp::connection_hdl>(hdl.lock().get(), hdl));
	if (!pWebserver->m_onOpenFun.empty())
	{
		pWebserver->m_onOpenFun(hdl.lock().get());
	}
}

static void on_close(WebSockServer* pWebserver, websocketpp::connection_hdl hdl)
{
	if (!pWebserver->m_onCloseFun.empty())
	{
		pWebserver->m_onCloseFun(hdl.lock().get(), "");
	}
	WriteLock writeLock(rwMutext);
	const ClientMap::iterator it = g_mapClient.find(hdl.lock().get());
	if (it != g_mapClient.end())
	{
		g_mapClient.erase(it);
		return;
	}
}

WebSockServer::WebSockServer()
{
	m_bServerStart = false;
}

WebSockServer& WebSockServer::Instance()
{
	static WebSockServer instance;
	return instance;
}

bool WebSockServer::Send(void* pClient, const std::string data, WsOpcode opcode)
{
	ReadLock readLock(rwMutext);
	const ClientMap::iterator it = g_mapClient.find(pClient);
	if (it == g_mapClient.end())
	{
		return false;
	}
	websocketpp::connection_hdl hdl = it->second;
	std::error_code ec;
	websocketpp::frame::opcode::value sCode = websocketpp::frame::opcode::BINARY;
	if (opcode == TEXT)
	{
		sCode = websocketpp::frame::opcode::TEXT;
	}
	g_server.send(hdl, data.c_str(), data.size(), sCode, ec);//发送二进制数据
	if (ec.value() == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void WebSockServer::Close(void* pClient)
{
	WriteLock writeLock(rwMutext);
	const ClientMap::iterator it = g_mapClient.find(pClient);
	if (it == g_mapClient.end())
	{
		return;
	}
	websocketpp::connection_hdl hdl = it->second;
	g_server.close(hdl, (websocketpp::close::status::value)0, "");
	g_mapClient.erase(it);
}

void WebSockServer::CloseAll()
{
	WriteLock writeLock(rwMutext);
	for (auto it = g_mapClient.begin(); it != g_mapClient.end(); it++)
	{
		websocketpp::connection_hdl hdl = it->second;
		g_server.close(hdl, (websocketpp::close::status::value)0, "");
	}
	g_mapClient.clear();
}

bool WebSockServer::Init(uint16_t uPort, OnOpenFun openFun, OnCloseFun closeFun, OnMessageFun msgFun, void* pOiService)
{
	m_onOpenFun = openFun;
	m_onCloseFun = closeFun;
	m_onMsgFun = msgFun;

	try {
		g_server.set_access_channels(websocketpp::log::alevel::none);
		g_server.clear_access_channels(websocketpp::log::alevel::none);
		if (pOiService == nullptr)
		{
			g_server.init_asio();
		}
		else
		{
			g_server.init_asio((boost::asio::io_service*)pOiService);
		}
		g_server.set_message_handler(bind(&on_message, this, ::_1, ::_2));
		g_server.set_close_handler(bind(&on_close, this, ::_1));
		g_server.set_open_handler(bind(&on_open, this, ::_1));
		g_server.set_reuse_addr(true);
		g_server.listen(websocketpp::lib::asio::ip::tcp::v4(), uPort);
		g_server.start_accept();
	}
	catch (websocketpp::exception const& e) {
		return false;
	}
	catch (...) {
		return false;
	}

	return true;
}

bool WebSockServer::StartServer()
{
	if (!m_bServerStart)
	{
		m_bServerStart = true;
		g_server.run();
	}

	return true;
}

void WebSockServer::StopServer()
{
	g_server.stop();
	m_bServerStart = false;
}

void WebSockServer::StopListening()
{
	g_server.stop_listening();
}

