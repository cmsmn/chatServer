#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "tcpConnection.h"
#include "json.hpp"

#include <unordered_map>
#include <functional>

using json = nlohmann::json;


using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>

//聊天服务器业务类(单例模式)
class ChatService
{
public:
	//单例模式示例接口	
	static ChatService* instane();

	//处理登录业务
	void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

	//处理注册任务
	void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);	

private:
	ChatService();

	//消息id和其对应的业务处理方法
	std::unordered_map<int, MsgHandler> msgHandlerMap;
};





#endif  
