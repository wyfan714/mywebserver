#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <iostream>
#include <string.h>
#include <string>
#include "../log/locker.h"
#include "../log/log.h"

using namespace std;

class connectionPool
{
public:
    MYSQL *getConnection();
    bool releaseConnection(MYSQL *conn);
    int getFreeConn();
    void destroyPool();

    static connectionPool *getInstance();
    void init(string url, string user, string passWord, string dataBaseName, int port, int maxConn);

private:
    connectionPool();
    ~connectionPool();
    int maxConnCount;  // 最大连接数
    int curConnCount;  // 当前已使用的连接数
    int freeConnCount; // 当前空闲的连接数
    locker lock;
    list<MYSQL *> connList; // 连接池
    sem reserve;

public:
    string connUrl;
    string connPort;
    string connUser;
    string connPassWord;
    string connDataBaseName;
};

#endif