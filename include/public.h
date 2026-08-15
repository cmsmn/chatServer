#ifndef PUBLIC_H
#define PUBLIC_H


#include <cstring>
#include <string>
#include <vector>
#include <arpa/inet.h> 

/*
 * server和client的公共文件
*/

enum EnMsgType
{
	LOGIN_MSG = 1, //登录消息
	LOGIN_MSG_ACK, //登录响应消息
	LOGINOUT_MSG,
	REG_MSG, //注册消息
	REG_MSG_ACK, //注册响应消息
	ONE_CHAT_MSG, //聊天消息
	ADD_FRIEND_MSG, //添加好友消息
	CREATE_GROUP_MSG, //创建群组
	ADD_GROUP_MSG, //加入群足
	GROUP_CHAT_MSG //群聊天	
};

#include <iostream>

namespace MessageProtocol
{

	const int u32Tysize = sizeof(uint32_t);

	const uint32_t MAGIC_NUMBER = 0x4D534700;

	std::string msgPack(const std::string &data)
	{
		uint32_t magic;      // 魔数，用于校验和协议识别，如0x1234
    	uint32_t payload_len; // 有效载荷长度（不含包头），网络字节序
		magic =  htonl(MAGIC_NUMBER);
		payload_len = htonl(data.size());

		std::string result;
		result.reserve(u32Tysize + u32Tysize + data.size());
		result.append(reinterpret_cast<const char*>(&magic), u32Tysize);
		result.append(reinterpret_cast<const char*>(&payload_len), u32Tysize);
		result.append(data);
		std::cout<< result <<std::endl;
        return result;
	}

	class Unpacker
	{
	public:

		void append(const char* data, size_t len)
		{
			buffer_.insert(buffer_.end(), data, data+len);
			while (parseOne()){}
		}

		std::string getMessage() 
		{
        	if (messages_.empty()) return "";
        	std::string msg = std::move(messages_.front());
        	messages_.erase(messages_.begin());
        	return msg;
    	}
	private:

		static const size_t HEADER_SIZE = 8;  // 包头长度

		bool parseOne()
		{
			if (buffer_.size() < HEADER_SIZE)
			{
				//接收的数据不够包头长度继续接收
				return false;
			}
			
			uint32_t magicNet, lenNet;
        	memcpy(&magicNet, buffer_.data(), u32Tysize);
        	memcpy(&lenNet, buffer_.data() + u32Tysize, u32Tysize);
        	uint32_t magic = ntohl(magicNet);
        	uint32_t bodyLen = ntohl(lenNet);
			if(magic != MAGIC_NUMBER)
			{
				buffer_.erase(buffer_.begin());
				return true;
			}

			if (buffer_.size() < (HEADER_SIZE + bodyLen))
			{
				return false;
			}
			
			//提取数据
			std::string msg(buffer_.begin() + HEADER_SIZE, 
					buffer_.begin() + HEADER_SIZE+ bodyLen);

			messages_.push_back(std::move(msg));
			//删除提取好的数据
			buffer_.erase(buffer_.begin(), buffer_.begin() + HEADER_SIZE+ bodyLen);

			return true;
		}

		std::vector<char> buffer_; //缓冲区
		std::vector<std::string> messages_; //数据容器
	
	};
}

#endif
