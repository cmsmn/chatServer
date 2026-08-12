#include "json.hpp"
#include "group.h"
#include "user.h"
#include "public.h"


#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <ctime>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using json = nlohmann::json;

//记录当前系统登录的用户信息
User g_currentUser;
//记录当前登录用户好友列表信息
std::vector<User> g_currentUserFriendList;
//记录代码获取用户登录的群组列表信息
std::vector<Group> g_currentUseGroupList;

//显示当前登录用户的基本信息
void showCurrentUserData();

//控制聊天页面和线程退出
bool isMianMrnuRunning = false;

//接受线程
void readTaskHandler(int CliendFd);
//获取系统时间
std::string getCurrentTime();
//主页面显示
void mainMenu(int clientFd);

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		std::cerr << "command invalid examlp:./ChatClient 127.0.0.1 4321" << std::endl;
		exit(1);
	}
	
	char *ip = argv[1];
	uint16_t port = atoi(argv[2]);

	//创建客户端的socket
	int clientFd = socket(AF_INET, SOCK_STREAM, 0);
	if(clientFd == -1)
	{
		std::cerr << "socket create error" << std::endl;
		exit(1);
	}
	
	//填写客户端连接服务器的ip端口信息
	sockaddr_in server;
	memset(&server, 0, sizeof(sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ip);

	//连接服务器
	if(connect(clientFd, (sockaddr*)&server, sizeof(sockaddr_in)))
	{
		std::cerr << "connect server error" << std::endl;
		close(clientFd);
		exit(1);
	}
	
	while(true)
	{
		//显示页面菜单 登录注册 退出
		std::cout << "=-=-=-=-=-=-=-=-=-=" << std::endl;
		std::cout << "1.log" << std::endl;
		std::cout << "2.register" << std::endl;
		std::cout << "1.quit" << std::endl;
		std::cout << "=-=-=-=-=-=-=-=-=-=" << std::endl;
		std::cout << "choice";
		int choice = 0;
		std::cin >> choice;
		std::cin.get(); //读取缓存区残留的回车
		
		switch(choice)
		{
		case 1:
		{
			int id = 0;
			char pwd[20] = {0};
			std::cout << "userid:";
			std::cin >> id;
			std::cin.get();
			std::cout << "userpassword:";
			std::cin.getline(pwd, 20);
		
			json js;
			js["msgid"] = LOGIN_MSG;
			js["id"] = id;
			js["password"] = pwd;
			std::string request = js.dump();
			
			//std::cout << request << std::endl;

			int len = send(clientFd, request.c_str(), strlen(request.c_str()) + 1, 0);
			if(len == -1)
			{
				std::cerr << "send reg msg error" << request << std::endl;
			}
			else
			{
				char buffer[1024] = {0};
				len = recv(clientFd, buffer, 1024, 0);
				if(len == -1)
				{
					std::cerr << "recv reg response error" << std::endl;
				}
				else
				{
					json responsejs = json::parse(buffer);
					if(responsejs["error"].get<int>() != 0)
					{
						std::cerr << responsejs["errmsg"] << std::endl;
					}
					else
					{
						//记录当前用户的 id name
						g_currentUser.setId(responsejs["id"].get<int>());
						g_currentUser.setName(responsejs["name"]);
						
						//记录当前用户好友列表信息
						if(responsejs.contains("friendMsg"))
						{
							std::vector<std::string> vec = responsejs["friendMsg"];
							for(std::string &str: vec)
							{
								json js = json::parse(str);
								User user;
								user.setId(js["id"].get<int>());
								user.setName(js["name"]);
								user.setStateString(js["state"]);
								g_currentUserFriendList.push_back(user);
							}
						}
						
						if(responsejs.contains("groups"))
						{
							std::vector<std::string> vec1 = responsejs["groups"];
							for(std::string &groupStr : vec1)
							{
								json grpjs = json::parse(groupStr);
								Group group;
								group.setId(grpjs["id"].get<int>());
								group.setName(grpjs["groupname"]);
								group.setDesc(grpjs["groupdesc"]);
								
								std::vector<std::string> vec2 = grpjs["users"];
								for(std::string &userStr : vec2)
								{
									GroupUser user;
									json js = json::parse(userStr);
									user.setId(js["id"].get<int>());
									user.setName(js["name"]);
									user.setStateString(js["state"]);
									user.setRole(js["role"]);
									group.getUsers().push_back(user);
								}
								g_currentUseGroupList.push_back(group);
							}
						}
						
						//显示登录用户的基本信息
						showCurrentUserData();
						
						//显示当前用户的离线消息
						if(responsejs.contains("offlineMsg"))
						{
							std::vector<std::string> vec = responsejs["offlineMsg"];
							for(std::string &str : vec)
							{
								json js = json::parse(str);
								int msgType = js["msgid"].get<int>();
								if(ONE_CHAT_MSG == msgType) //一对一
								{
									std::cout << js["time"].get<std::string>() 
											<< " [" << js["id"] << "] " 
											<< js["name"].get<std::string>()
											<< " said:" << js["msg"].get<std::string>() << std::endl;
								}
								else //群组消息
								{
									std::cout << "群消息[" << js["groupid"] << "]:"
											<< js["time"].get<std::string>() 
											<< " [" << js["id"] << "] " 
											<< js["name"].get<std::string>()
											<< " said:" << js["msg"].get<std::string>() << std::endl;
								}
							}
						}
						
						//登录成功 启动接受线程负责接受数据
						//防止线程多次创建 
						static int threadNumber = 0;
						if(threadNumber > 1)
						{
							std::thread readTask(readTaskHandler, clientFd);//创建线程
							readTask.detach();//设置分离线程
							threadNumber++;
						}
						isMianMrnuRunning = true;
						//进入聊天主菜单
						mainMenu(clientFd);
					}
				}
			} 
		}
		break;
		case 2:
		{
			//注册
			char name[20] = {0};
			char pwd[20] = {0};
			std::cout << "username:" ;
			std::cin.getline(name, 20);
			std::cout << "userpassword" ;
			std::cin.getline(pwd, 20);
				
			//json数据组装
			json js;
			js["msgid"] = REG_MSG;
			js["name"] = name;
			js["password"] = pwd;
			std::string request = js.dump();
			
			//向服务器发送数据
			int len = send(clientFd, request.c_str(), strlen(request.c_str()) + 1, 0);
			if(len == -1)
			{
				std::cerr << "send reg msg error" << request << std::endl;
			}
			else
			{
				char buffer[1024] = {0};
				len = recv(clientFd, buffer, 1024, 0);
				if(len == -1)
				{
					std::cerr << "recv reg response error" << std::endl;
				}
				else
				{
					json responsejs = json::parse(buffer);
					if(responsejs["error"].get<int>() != 0)
					{
						std::cerr << name << " is already exist register error" << std::endl;
					}
					else
					{
						std::cout<< name << " register success userid is " << responsejs["id"]
							<< ", do not forget it!" <<std::endl;
					}
				}
			}
		}
		break;
		case 3:
			//退出
			close(clientFd);
			exit(0);
		defauit:
			std::cerr << "invalid input" << std::endl;
			break;
		}
	}
	return 0;
}


//显示当前登录用户的基本信息
void showCurrentUserData()
{
	std::cout << "=-=-=-=-=-=-=-=-=-=-login user-=-=-=-=-=-=-=-=-=-=" << std::endl;
	std::cout << "current login user=id:" << g_currentUser.getId() 
			<< " name" << g_currentUser.getName() <<std::endl;
	
	std::cout << "____________________friend list____________________" << std::endl;
	if(!g_currentUserFriendList.empty())
	{
		for(User &user : g_currentUserFriendList)
		{
			std::cout << user.getId() << " " << user.getName() << " " << user.getState() << std::endl;
		}
	}
	std::cout << "____________________group list____________________" << std::endl;
	if(!g_currentUseGroupList.empty())
	{
		for(Group &group : g_currentUseGroupList)
		{
			std::cout << group.getId() << " " << group.getName() << " " << group.getDesc() << std::endl;
			for(GroupUser &user : group.getUsers())
			{
				std::cout << user.getId() << " " << user.getName() << " " << user.getState() 
					<< " "  << user.getRole() << std::endl;
			}
		}
	}
	std::cout << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
}

//help command handler
void help(int fd = 0, std::string = "");

//chat command handler
void chat(int, std::string);

//addfriend command handler
void addfriend(int, std::string);

//creategroup command handler
void creategroup(int, std::string);

//addgroup command handler
void addgroup(int, std::string);

//groupchat command handler
void groupchat(int, std::string);

// loginout command handler
void loginout(int, std::string);

//系统支持的客户端命令列表
std::unordered_map<std::string, std::string> commandMap =
{
	{"help", "显示所有支持命令 help"},
	{"chat", "一对一聊天 chat:friendId:message"},
	{"addfriend", "添加好友 addfriend:friendId"},
	{"creategroup", "创建群组 creategroup:groupname:groupdesc"},
	{"addgroup", "加入群组 addgroup:groupId"},
	{"groupchat", "群组聊天 groupchat:groupId:messages"},
	{"loginout", "注销 loginout"}
};

//注册系统支持的客户端命令处理
std::unordered_map<std::string, std::function<void(int, std::string)>> commandHandlerMap = 
{
	{"help", help},
	{"chat", chat},
	{"addfriend", addfriend},
	{"creategroup", creategroup},
	{"addgroup", addgroup},
	{"groupchat", groupchat},
	{"loginout", loginout}
};

//主页面程序
void mainMenu(int clientFd)
{
	help();
	
	char buffer[1024] = {0};
	while(isMianMrnuRunning)
	{
		std::cin.getline(buffer, 1024);
		std::string commandBuf(buffer);
		std::string command; //存储命令
		int idx = commandBuf.find(":");
		if(idx == -1)
		{
			command = commandBuf;
		}
		else
		{
			command = commandBuf.substr(0, idx);
		}
		auto it = commandHandlerMap.find(command);
		if(it == commandHandlerMap.end())
		{
			std::cerr << "invalid input command" << std::endl;
			continue;
		}
		
		//调用命令key对应的函数对象 使用commandHandlerMap 命令和函数对象对应 可以保证开闭原则
		it->second(clientFd, commandBuf.substr(idx + 1, (commandBuf.size() - idx)));
	}
}

void help(int fd,std::string cmdMsg)
{
	std::cout << "show command list >>>" << std::endl;
	for(auto &p : commandMap)
	{
		std::cout << p.first << " : " << p.second << std::endl;
	}
	std::cout << std::endl;
}

//添加好友
void addfriend(int clientFd, std::string cmdMsg)
{
	int friendId = atoi(cmdMsg.c_str());
	json js;
	js["msgid"] = ADD_FRIEND_MSG;
	js["id"] = g_currentUser.getId();
	js["friendid"] = friendId;
	std::string buffer = js.dump();
	
	int len = send(clientFd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
	if(len == -1)
	{
		std::cerr << "send addfriend msg error -> " << buffer << std::endl;
	}
}

//一对一聊天
void chat(int clientFd, std::string cmdMsg)
{
	int idx = cmdMsg.find(":");
	if(idx == -1)
	{
		std::cerr << "chat command invalid " << std::endl;
		return;
	}

	int friendId = atoi(cmdMsg.substr(0, idx).c_str());
	std::string message = cmdMsg.substr(idx + 1, cmdMsg.size() - idx);

	json js;
	js["msgid"] = ONE_CHAT_MSG;
	js["id"] = g_currentUser.getId();
	js["name"] = g_currentUser.getName();
	js["toid"] = friendId;
	js["msg"] = message;
	js["time"] = getCurrentTime();
	std::string buffer = js.dump();
            
	int len = send(clientFd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
	if(len == -1)
	{   
		std::cerr << "send chat msg error -> " << buffer << std::endl;
	}
}

//创建群组
void creategroup(int clientFd, std::string cmdMsg)
{
	int idx = cmdMsg.find(":");
	if(idx == -1)
	{
		std::cerr << "creategroup command invalid " << std::endl;
		return;
	}
	std::string groupName = cmdMsg.substr(0, idx);
	std::string groupDesc = cmdMsg.substr(idx + 1, cmdMsg.size() - idx) ;
	
	json js;
	js["msgid"] = CREATE_GROUP_MSG;
	js["id"] = g_currentUser.getId();
	js["groupname"] = groupName;
	js["groupdesc"] = groupDesc;

	std::string buffer = js.dump();
            
	int len = send(clientFd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
	if(len == -1)    
	{                
		std::cerr << "send creategroup msg error -> " << buffer << std::endl;
	}
}

//添加群组
void addgroup(int clientFd, std::string cmdMsg)
{

	int groupId = atoi(cmdMsg.c_str());
	json js;
	js["msgid"] = ADD_GROUP_MSG;
	js["id"] = g_currentUser.getId();
	js["groupid"] = groupId;

	std::string buffer = js.dump();      
	int len = send(clientFd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
	if(len == -1)    
	{                
		std::cerr << "send addgroup msg error -> " << buffer << std::endl;
	}
}

//群组聊天
void groupchat(int clientFd, std::string cmdMsg)
{
	int idx = cmdMsg.find(":");
	if(idx == -1)
	{
		std::cerr << "creategroup command invalid " << std::endl;
		return;
	}

	int groupId = atoi(cmdMsg.substr(0, idx).c_str());
	std::string  groupMessages = cmdMsg.substr(idx + 1, cmdMsg.size() - idx);

	json js;
	js["msgid"] = GROUP_CHAT_MSG;
	js["id"] = g_currentUser.getId();
	js["name"] = g_currentUser.getName();
	js["groupid"] = groupId;
	js["msg"] = groupMessages;
	js["time"] = getCurrentTime();


	std::string buffer = js.dump();      
	int len = send(clientFd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
	if(len == -1)    
	{                
		std::cerr << "send groupchat msg error -> " << buffer << std::endl;
	}
}

//登出
void loginout(int clientFd, std::string cmdMsg)
{
	json js;
	js["msgid"] = LOGINOUT_MSG;
	js["id"] = g_currentUser.getId();
	js["time"] = getCurrentTime();


	std::string buffer = js.dump();      
	int len = send(clientFd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
	if(len == -1)    
	{                
		std::cerr << "send groupchat msg error -> " << buffer << std::endl;
	}
	isMianMrnuRunning = false;
	//退出把用户的缓存初始化
	g_currentUserFriendList.clear();
	g_currentUseGroupList.clear();

}

std::string getCurrentTime()
{   
	auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}

//接收数据线程
void readTaskHandler(int clientFd)
{
	//std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << std::endl;
	//线程recv堵塞导致线程释放不了 
	//多次登录导致线程多次创建
	//解决方法线程创建一次就行
	while(true)
	{
		char buffer[1024] = {0};
		int len = recv(clientFd, buffer, 1024, 0);
		if(len == -1 || 0 == len)
		{
			close(clientFd);
			exit(-1);
		}

		std::cout << buffer << std::endl;

		json js = json::parse(buffer);
		int msgType = js["msgid"].get<int>();
		if(ONE_CHAT_MSG == msgType) //一对一
		{
			std::cout << js["time"].get<std::string>() 
					<< " [" << js["id"] << "] " 
					<< js["name"].get<std::string>()
					<< " said:" << js["msg"].get<std::string>() << std::endl;
			continue;
		}
		
		if(GROUP_CHAT_MSG == msgType) //群组消息
		{
			std::cout << "群消息[" << js["groupid"] << "]:"
					<< js["time"].get<std::string>() 
					<< " [" << js["id"] << "] " 
					<< js["name"].get<std::string>()
					<< " said:" << js["msg"].get<std::string>() << std::endl;
			continue;
		}
	}
}








