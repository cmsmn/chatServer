#include "chatServer.h"
#include "json.hpp"


#include <functional>

using json = nlohmann::json; 

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
 
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
	if(conn->connected())
	{
		
	}
	else
	{
		//连接失败直接回收fd
		conn->shutdown();
	} 
}

void ChatServer::onMessage(const TcpConnectionPtr &conn
		,Buffer *buffer
		,Timestamp time)
{
	//从缓冲区读取数据放进字符串
	std::string buf = buffer->retrieveAllAsString();
	//数据的反序列化
	json js = json::parse(buf);
	//达到的目的完全解耦网络模块的代码和业务代码
	//js["msgid"]获取->业务handler->conn js time 
}
