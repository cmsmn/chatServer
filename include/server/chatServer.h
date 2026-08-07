#ifndef CHATSERVER_H
#define CHATSERVER_H

#include "tcpServer.h"
#include "eventLoop.h"

#include <string>

//聊天服务器的主类
class ChatServer
{
public:
	ChatServer(EventLoop *loop
		,const InetAddress &addr
		,const std::string &name
		,const int threadNum);

	void start(){ server_.start(); };
private:
	//上报联系相关信息的回调函数
	void onConnection(const TcpConnectionPtr &conn);
	
	//上报读写事件相关信息的回调函数
	void onMessage(const TcpConnectionPtr &conn 
		,Buffer *buffer
		,Timestamp time);


	TcpServer server_;
	EventLoop *loop_;
};



#endif
