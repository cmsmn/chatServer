#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>

#include <thread>
#include <functional>
#include <string>

class  Redis
{
public:
    Redis();
    ~Redis();
    
    //连接redis服务器
    bool connect();

    //向redis指定通道channel发布消息
    bool publish(int channel, std::string message);

    //向redis指定通道subscribe订阅消息
    bool subscribe(int channel);

    //向redis指定通道unsubscribe取消订阅消息
    bool unsubscribe(int channel);

    //在独立线程中接收订阅通道消息
    void observerChannelMessage();

    //初始化向业务层上报通道消息的回调对象
    void initNotifyHandler(std::function<void(int, std::string)> fn);
private:
    //subscribe订阅会堵塞 所以一个上下文无法处理发布和订阅的事件

    //hiredis同步上下文对象 负责 publish 消息
    redisContext* publishContext_;
    
    //hiredis同步上下文对象 负责 subscribe 消息
    redisContext* subcribeContext_;

    //回调操作 收到订阅的消息 给service层上报
    std::function<void(int, std::string)> notifyMessageHandler;
};




#endif 