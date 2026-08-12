#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "tcpConnection.h"

#include "json.hpp"
#include "userModel.h"
#include "offlineMessageModel.h"
#include "firendModel.h"
#include "groupModel.h"

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

	//添加好友业务
	void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
	
	//创建群组业务
	void creadteGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

	//加入群组业务
	void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

	//群组聊天业务
	void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

	//退出业务
	void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);

	//获取消息处理器
	MsgHandler getMsgHandler(int msgId);

	//服务器异常 业务重置方法
	void reset();
	
	//处理用户异常退出	
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
	//用户信息操作对象
	UserModel userModel_;
	//好友信息操作对象
	FriendModel friendModel_;
	//群组信息操作对象
	GroupModel groupModel_;
	//离线信息存储
	OfflineMsgModel offlineMsgModel_;
	
};





#endif  
