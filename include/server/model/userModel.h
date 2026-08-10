#ifndef USERMODEL_H
#define	USERMODEL_H

#include <user.h>

class UserModel
{
public:
	//增加方法
	bool insert(User &user);

	//根据用户id查询选项
	User query(int id);

	//更新用户的状态信息
	bool updateState(User user);

	//重置用户的状态信息
	void resetState();	
};

#endif
