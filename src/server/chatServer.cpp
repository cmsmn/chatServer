#include "chatServer.h"
#include "json.hpp"
#include "chatService.h"

#include <functional>

using json = nlohmann::json; 

//构造函数
ChatServer::ChatServer(EventLoop *loop
		,const InetAddress &addr
		,const std::string &name
		,const int threadNum)
	:server_(loop, addr, name)
	,loop_(loop)
{
	server_.setConnectionCallback(
		std::bind(&ChatServer::onConnection
		,this
		,std::placeholders::_1));

	server_.setMessageCallback(
		std::bind(&ChatServer::onMessage
		,this
		,std::placeholders::_1
		,std::placeholders::_2
		,std::placeholders::_3));
	
	server_.setThreadNum(threadNum);	
}

 //上报联系相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
	if(conn->connected())
	{
		
	}
	else
	{
		//客户端异常退出处理
		ChatService::instance()->clientCloseException(conn);
		//连接失败直接回收fd
		conn->shutdown();
	} 
}

//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn
		,Buffer *buffer
		,Timestamp time)
{
	//从缓冲区读取数据放进字符串
	std::string buf = buffer->retrieveAllAsString();
	//数据的反序列化
	std::cout << buf << std::endl; ;
	json js = json::parse(buf);
	//达到的目的完全解耦网络模块的代码和业务代码
	//js["msgid"]获取->业务handler->conn js time 
	auto magHandler = ChatService::instance()->getMsgHandler(js["msgid"].get<int>());
	magHandler(conn, js, time);
}
