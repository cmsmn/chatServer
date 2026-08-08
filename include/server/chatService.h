#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "tcpConnection.h"
#include "json.hpp"

#include <unordered_map>
#include <functional>

using json = nlohmann::json;

//处理消息事件的回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

//聊天服务器业务类(单例模式)
class ChatService
{
public:

	//单例模实例接口	
	static ChatService* instance();

	//处理登录业务
	void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

	//处理注册任务
	void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);	

	//获取消息处理器
	MsgHandler getMsgHandler(int msgId);


private:
	//单例构造函数注册消息以及对应的回调操作
	ChatService();

	//消息id和其对应的业务处理方法
	std::unordered_map<int, MsgHandler> msgHandlerMap_;
};





#endif  
