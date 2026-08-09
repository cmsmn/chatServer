#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "tcpConnection.h"

#include "json.hpp"
#include "userModel.h"
#include "offlineMessageModel.h"

#include <unordered_map>
#include <functional>
#include <mutex>

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

	//一对一聊天业务
	void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

	//获取消息处理器
	MsgHandler getMsgHandler(int msgId);
	
	void clientCloseException(const TcpConnectionPtr &conn);
private:
	//单例构造函数注册消息以及对应的回调操作
	ChatService();

	//消息id和其对应的业务处理方法
	std::unordered_map<int, MsgHandler> msgHandlerMap_;
	
	//存储在线用户的通信连接在运行中会被各种线程调用 所以要线程安全
	std::unordered_map<int, TcpConnectionPtr> userConnMap_;

	//定义互斥锁保证 记录连接容器线程安全
	std::mutex connMutex_;
	
	//数据操作类对象
	UserModel userModel_;
	//离线消息存储
	OfflineMsgModel offlineMsgModel_;
	
};





#endif  
