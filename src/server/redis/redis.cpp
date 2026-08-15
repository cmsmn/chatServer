#include "redis.h"

#include <iostream>

Redis::Redis()
    :publishContext_(nullptr)
    ,subcribeContext_(nullptr)
{}

Redis::~Redis()
{
    if(publishContext_ != nullptr)
    {
        redisFree(publishContext_);
    }
    if(subcribeContext_ != nullptr)
    {
        redisFree(subcribeContext_);
    }
}

bool Redis::connect()
{

    //连接redis服务器
    publishContext_ = redisConnect("127.0.0.1", 6379);
    if(publishContext_ == nullptr)
    {
        std::cerr << "connect redis failed" << std::endl;
        return false;
    }

    subcribeContext_ = redisConnect("127.0.0.1", 6379);
    if(subcribeContext_ == nullptr)
    {
        std::cerr << "connect redis failed" << std::endl;
        return false;
    }

    //因为subscribe订阅消息是堵塞的 所以要创建线程独立接收数据 
    std::thread t (
        [&]
        {
            observerChannelMessage();
        }
    );
    //设置守护线程
    t.detach();

    std::cerr << "connect redis_server success!" << std::endl;
    return true;
}

bool Redis::publish(int channel, std::string message)
{

    //redisCommand向redis发送命令
    //redisCommand在向redis发送命令会堵塞等待消息的返回
    //但是如果执行了一个本来就堵塞的消息 如消息监听通道队列 会导致程序都堵塞在这里
    redisReply *reply = (redisReply*)redisCommand(publishContext_, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr)
    {
        std::cerr << "publish command failed" << std::endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel)
{
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源
    if(REDIS_ERR == redisAppendCommand(subcribeContext_, "SUBSCRIBE %d", channel))
    {
        std::cerr << "subscribe  command failed" << std::endl;
        return false;
    }

    int done = 0;
    while (!done)
    {
        if(REDIS_ERR == redisBufferWrite(subcribeContext_, &done))
        {
            std::cerr << "subscribe  command failed" << std::endl;
            return false;
        }
    }

    return true;
}

bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(subcribeContext_, "UNSUBSCRIBE %d", channel))
    {
        std::cerr << "unsubscribe command failed!" << std::endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(subcribeContext_, &done))
        {
            std::cerr << "unsubscribe command failed!" << std::endl;
            return false;
        }
    }
    return true;
}

void Redis::observerChannelMessage()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(subcribeContext_, (void**)&reply))
    {
        if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            //给业务层上报通道上发生的消息
            notifyMessageHandler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
     std::cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << std::endl;
}

void Redis::initNotifyHandler(std::function<void(int,std::string)> fn)
{
    notifyMessageHandler = fn;
}
