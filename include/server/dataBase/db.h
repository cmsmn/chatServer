#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
#include "log.h"

static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "241018@Pp";
static std::string dbname = "chat";


class MySQL
{
public:
	//初始化数据库连接
	MySQL()
	{
		conn_ = mysql_init(nullptr);
	}
	
	//释放数据库连接资源
	~MySQL()
	{
		if(conn_ != nullptr)
		{
			mysql_close(conn_);
		}
	}
	
	//连接数据库
	bool connect()
	{
		MYSQL *p = mysql_real_connect(conn_
			,server.c_str(), user.c_str()
			,password.c_str(), dbname.c_str()
			,3306, nullptr, 0);

		if(p != nullptr)
		{
			// c/cpp代码默认是ASCII 不设置在mysql拉下来的中文会有乱码
			mysql_query(conn_, "set names gbk");
			LOG_INFO("connect mysql success \n");
		}
		else
		{
			LOG_ERROR("connect mysql fail \n");
		}

		return p;
	}
	
	//更新操作
	bool update(std::string sql)
	{
		if(mysql_query(conn_, sql.c_str()))
		{
			LOG_ERROR("%s 语句 更新失败", sql.c_str());
			return false;
		}
		return true;
	}
	
	//查询操作
	MYSQL_RES* query(std::string sql)
	{
		
		if(mysql_query(conn_, sql.c_str()))
		{
			LOG_ERROR("%s 语句 查询失败", sql.c_str());
			return nullptr;
		}      
		return mysql_use_result(conn_);
	}

	//获取mysql连接
	MYSQL* getConnection() { return conn_; }		
private:
	MYSQL* conn_;
};



#endif
