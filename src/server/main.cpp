#include "chatServer.h"
#include "chatService.h"

#include <iostream>
#include <signal.h>

// 处理服务器 ctrl+c结束后用户状态问题
void resetHandler(int)
{
	ChatService::instance()->reset();
	exit(0);
}

int main()
{

	signal(SIGINT, resetHandler);

	EventLoop loop;
	InetAddress addr(4321);
	ChatServer server(&loop, addr, "chatServer", 4);
	server.start();
	loop.loop();
	return 0;
}
