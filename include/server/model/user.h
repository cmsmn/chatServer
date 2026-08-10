#ifndef USER_H
#define USER_H

#include <string>

//User表的ORM类
class User
{
public:
	User(int id = -1
		,std::string name = ""
		,std::string pwd = ""
		,std::string state = "offline")
	:id_(id)
	,name_(name)
	,password_(pwd)
	,state_(state)
	{}

	void setId(int id) { id_ = id; }
	void setName(std::string name) { name_ = name; }
	void setPassword(std::string pwd) { password_ = pwd; }
	void setStateString(std::string state) { state_ =state;}
	void setStateBool(bool no)	
	{
		if(no)
		{
			state_ = "online";
		}
		else
		{
			state_ = "offline";
		}
	}
	

	int getId() { return id_; }
	std::string getName() { return name_; }
	std::string getPassword() { return password_; }
	std::string getState() { return state_;}

private:
	int id_;
	std::string name_;
	std::string password_;
	std::string state_;
};

#endif
