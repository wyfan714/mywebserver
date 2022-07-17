#ifndef CONFIG_H
#define CONFIG_H

#include "../webserver.h"
using namespace std;

class Config
{

public:
    Config();
    ~Config(){};

    // 端口号
    int port;
    // 异步日志or同步日志
    bool is_sync_log;
    // listenfd是否是LT
    bool listen_is_LT;
    // connfd触发模式是否是LT
    bool conn_is_LT;
    // 优雅, 实在是太优雅了的关闭连接, true是优雅, false不优雅
    bool is_opt_linger;
    // 数据库连接池数量
    int conn_count;
    // 线程池线程数量
    int thread_count;
    // 是否关闭日志
    bool is_close_log;
    // proactor or reactor 原则上可以换bool,但是模型可能会增加..
    int actor_model;

    // 定时器类型 0为升序链表 1为时间轮 2为时间堆 暂时没用上
    // int timer_type;

    // 解析参数
    void parse_args(int argc, char *argv[]);
};

#endif