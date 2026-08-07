#include "chatServer.h"
#include <iostream>

int main()
{
	EventLoop loop;
	InetAddress addr(4321);
	ChatServer server(&loop, addr, "chatServer", 4);
	server.start();
	loop.loop();
	return 0;
}
