#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.h"
#include "db.h"

#include <string>
#include <vector>

//组信息操作方法接口
class GroupModel
{
public:
	//创建群组
	bool createGroup(Group &group);

	//加入群组
	void addGroup(int userId, int groupId, std::string role);
	
	//查询用户所在群组消息
	std::vector<Group> queryGroup(int userId);
	
	//根据指定的groupId查询群组所有用户的id列表 除useid自己 
	//主要用户群聊业务给群组其他成员群发消息
	std::vector<int> queryGroupUsers(int userId, int groupId);
};
#endif
