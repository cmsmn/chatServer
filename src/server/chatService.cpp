#include "chatService.h"
#include "public.h"

#include "log.h"
#include "timestamp.h"


//单例模式实例接口
ChatService* ChatService::instance()
{
	static ChatService service;
	return &service;
}

//单例构造函数注册消息以及对应的回调操作
ChatService::ChatService()
{
	//将消息的id和业务处理函数对应绑定 
	//这样就可以让网络模块和业务模块解耦
	msgHandlerMap_.insert({LOGIN_MSG, std::bind(
		&ChatService::login
		,this
		,std::placeholders::_1
		,std::placeholders::_2
		,std::placeholders::_3)});

	msgHandlerMap_.insert({REG_MSG, std::bind(
		&ChatService::reg		
		,this
		,std::placeholders::_1
		,std::placeholders::_2
		,std::placeholders::_3)});
}

//获取消息处理器
MsgHandler ChatService::getMsgHandler(int msgId)
{
	auto it = msgHandlerMap_.find(msgId);
	if(it == msgHandlerMap_.end())
	{
		//如果获取了一个根本没注册的信号id就返回一个默认的
		//处理器然后输出err日记
		return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
		{
			LOG_ERROR("msgid: %d can not find handler \n", msgId);
		};
	} 
	else
	{
		return msgHandlerMap_[msgId];
	}
}

//处理登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
	LOG_INFO("do login service \n");
}

//处理注册任务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
	LOG_INFO("do reg service \n");
}
