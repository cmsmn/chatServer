#include "chatService.h"
#include "public.h"

#include "log.h"
#include "timestamp.h"

#include <vector>
#include <string>

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
	//登录消息
	msgHandlerMap_.insert({LOGIN_MSG, std::bind(
		&ChatService::login
		,this
		,std::placeholders::_1
		,std::placeholders::_2
		,std::placeholders::_3)});
	//注册消息
	msgHandlerMap_.insert({REG_MSG, std::bind(
		&ChatService::reg		
		,this
		,std::placeholders::_1
		,std::placeholders::_2
		,std::placeholders::_3)});
	//聊天消息
	msgHandlerMap_.insert({ONE_CHAT_MSG, std::bind(
		&ChatService::oneChat
		,this
		,std::placeholders::_1
		,std::placeholders::_2
		,std::placeholders::_3)});
	//添加好友消息
	msgHandlerMap_.insert({ADD_FRIEND_MSG, std::bind(
		&ChatService::addFriend
		,this
		,std::placeholders::_1
		,std::placeholders::_2
		,std::placeholders::_3)});

	//加入群组
	msgHandlerMap_.insert({ADD_GROUP_MSG, std::bind(
		&ChatService::addGroup
		,this
		,std::placeholders::_1
		,std::placeholders::_2
		,std::placeholders::_3)});

	//群聊天
	msgHandlerMap_.insert({GROUP_CHAT_MSG, std::bind(
		&ChatService::groupChat
		,this
		,std::placeholders::_1
		,std::placeholders::_2
		,std::placeholders::_3)});

	//创建群组
	msgHandlerMap_.insert({CREATE_GROUP_MSG, std::bind(
		&ChatService::creadteGroup
		,this
		,std::placeholders::_1
		,std::placeholders::_2
		,std::placeholders::_3)});

		//客户端退出
	msgHandlerMap_.insert({LOGINOUT_MSG, std::bind(
		&ChatService::loginout
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

	//std::string request = js.dump();
	//std::cout << request << std::endl; 

	int id = js["id"].get<int>();
	std::string pwd = js["password"];

	User user = userModel_.query(id);
 	json  response;
	if(user.getId() == id  && user.getPassword() == pwd)
	{
		if(user.getState() == "online")
		{
			//该用户已经登录 不容许重复登录
			response["msgid"] = LOGIN_MSG_ACK;
			response["error"] = 2;
			response["errmsg"] = "重复登录";
			conn->send(response.dump());
		}
		else
		{
			//登录成功 
			//更新用户状态
			//数据库的CRUD由mysql保证线程安全
			user.setStateBool(true);
			userModel_.updateState(user);
			{
				//记录用户连接消息
				//连接容器保证线程安全
				std::lock_guard<std::mutex> lock(connMutex_);
				userConnMap_.insert({id, conn});
			}
			
			//返回用户消息
			response["msgid"] = LOGIN_MSG_ACK;
			response["error"] = 0;
			response["id"] = user.getId();
			response["name"] = user.getName();
			//查询用户是否有离线消息
			std::vector<std::string> vec = offlineMsgModel_.query(id);
			if(!vec.empty())
			{
				//有离线消息
				response["offlineMsg"] = vec;
				//读取完消息删除
				offlineMsgModel_.remove(id);
			}
			//查询该用户的好友信息并返回
			std::vector<User> userVec = friendModel_.query(id);
			if(!userVec.empty())
			{
				std::vector<std::string> friendUserVec;
				for(User &user : userVec)
				{
					json js;
					js["id"] = user.getId();
					js["name"] = user.getName();
					js["state"] = user.getState();
					friendUserVec.push_back(js.dump()); 
				}
				response["friendMsg"] =  friendUserVec;
			}
			
			//查询用户群组信息
			std::vector<Group> groupUserVec = groupModel_.queryGroup(id);
			if(!groupUserVec.empty())
			{
				std::vector<std::string> groupV;
				for(Group &group : groupUserVec)
				{
					json grpjson;
					grpjson["id"] = group.getId();
					grpjson["groupname"] = group.getName();
					grpjson["groupdesc"] = group.getDesc();
					std::vector<std::string> userV;
					for(GroupUser &user : group.getUsers())
					{
						
						json js;
						js["id"] = user.getId();
						js["name"] = user.getName();
						js["state"] = user.getState();
						js["role"] = user.getRole();
						userV.push_back(js.dump());
					}
					grpjson["users"] = userV;
					groupV.push_back(grpjson.dump());
				}
				response["groups"] = groupV;
			}
			//dump方法将json 转化为字符串
			conn->send(response.dump());
		}
	}
	else
	{
		//登录失败
		response["msgid"] = LOGIN_MSG_ACK;
		response["error"] = 1;
		response["errmsg"] = "用户名或者密码错误";
		conn->send(response.dump());
	}
}

//处理注册任务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
	std::string name = js["name"];
	std::string pwd = js["password"];

	User user;
	user.setName(name);
	user.setPassword(pwd);

	if(userModel_.insert(user))
	{
		//注册成功
		json  response;
		response["msgid"] = REG_MSG_ACK;
		response["error"] = 0;
		response["id"] = user.getId();
		//dump方法将json 转化为字符串
		conn->send(response.dump());
		
	}
	else
	{
		//注册失败         
		json  response;      
		response["msgid"] = REG_MSG_ACK;
		response["error"] = 1;
		//dump方法将json 转化为字符串
		conn->send(response.dump());
	}
	

}

//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
	// json fromid fromName toid msg

	int toId = js["toid"].get<int>();
	{
		std::lock_guard<std::mutex> lock(connMutex_);
		auto it = userConnMap_.find(toId);
		if(it != userConnMap_.end())
		{
			//服务器从连接容器找到toid的连接 然后推送msg
			//要通过conn转发所以要保证线程安全
			//有粘包问题
			it->second->send(js.dump() + "\n");
			return;
		}
	}
	//toid不在线 存储离线信息
	offlineMsgModel_.insert(toId, js.dump());
}

//添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
	int userid = js["id"].get<int>();
	int friendId = js["friendid"].get<int>();

	if((userModel_.query(friendId).getId()) == -1)
	{
		json  response;
		response["msgid"] = ADD_FRIEND_MSG;
		response["error"] = 3;
		response["errmsg"] = "添加的用户不存在";
		conn->send(response.dump());
		return;
	}
	//存储好友信息
	friendModel_.insert(userid, friendId);
}

//创建群组任务
void ChatService::creadteGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
	int userId = js["id"].get<int>();
	std::string name = js["groupname"];
	std::string desc = js["groupdesc"];
	Group group(-1, name, desc);
	if(groupModel_.createGroup(group))
	{
		//存储创始人信息
		groupModel_.addGroup(userId, group.getId(), "creator");
	}
}

//加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
	int userid = js["id"].get<int>();
	int groupId = js["groupid"].get<int>();
	groupModel_.addGroup(userid, groupId, "normal");
}

//群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
 	int userId = js["id"].get<int>();
	int groupId = js["groupid"].get<int>();
	std::vector<int> useridVer = groupModel_.queryGroupUsers(userId, groupId);
	{
		std::lock_guard<std::mutex> lock(connMutex_);
		for(int id : useridVer)
		{
			auto it = userConnMap_.find(id);
			if(it != userConnMap_.end())
			{
				it->second->send(js.dump());
			}
			else
			{
				offlineMsgModel_.insert(id, js.dump());
			}
		}
	}
}

//处理登出业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
	int userId = js["id"].get<int>();
	{
		std::lock_guard<std::mutex> lock(connMutex_);
		auto it  = userConnMap_.find(userId);
		if(it != userConnMap_.end())
		{
			userConnMap_.erase(it);
		}
	}
	User user;
	user.setId(userId);
	user.setStateBool(false);
	userModel_.updateState(user);
}

//处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{	
	User user;
	{
		std::lock_guard<std::mutex> lock(connMutex_);
		for(auto it = userConnMap_.begin(); it != userConnMap_.end(); ++it)
		{
			if(it->second == conn)
			{
				//从连接表删除用户连接消息
				user.setId(it->first);
				userConnMap_.erase(it);
				break;
			}
		}
	}
	//更新用户的状态信息
	if(user.getId() != -1)
	{
		user.setStateBool(false);
		userModel_.updateState(user);
	}
}

void ChatService::reset()
{
	//把在线的用户 设置为下线
	userModel_.resetState();
}
















