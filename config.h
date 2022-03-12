#ifndef CONFIG_H
#define CONFIG_H

using namespace std;

class Config
{

public:
    Config();
    ~Config(){};

    // 端口号
    int PORT;

    // listenfd触发模式 0为LT 1为ET
    int LISTENTYPE;

    // connfd触发模式 0为LT 1为ET
    int CONNTYPE;

    // 日志写入方式 0为同步写入 1为异步写入
    int LOGTYPE;

    // 定时器类型 0为升序链表 1为时间轮 2为时间堆
    int TIMERTYPE;

    // 解析参数
    void parse_args(int argc, char *argv[]);
};

#endif