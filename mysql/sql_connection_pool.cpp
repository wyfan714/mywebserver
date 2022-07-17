#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

using namespace std;

connectionPool::connectionPool()
{
    curConnCount = 0;
    freeConnCount = 0;
}
connectionPool *connectionPool::getInstance()
{
    // lazy式, 需要时创建
    static connectionPool connPool;
    return &connPool;
}

void connectionPool::init(string url, string user, string passWord, string dataBaseName, int port, int maxConn)
{
    connUrl = url;
    connUser = user;
    connPort = port;
    connPassWord = passWord;
    for (int i = 0; i < maxConn; i++)
    {
        MYSQL *conn = nullptr;
        conn = mysql_init(conn);
        if (conn == nullptr)
        {
            LOG_ERROR("Mysql init Error");
            exit(1);
        }
        conn = mysql_real_connect(conn, url.c_str(), user.c_str(), passWord.c_str(), dataBaseName.c_str(), port, NULL, 0);
        if (conn == NULL)
        {
            LOG_ERROR("Mysql real connect Error");
            exit(1);
        }
        connList.push_back(conn);
        ++freeConnCount;
    }
    reserve = sem(freeConnCount);
    maxConnCount = freeConnCount;
}

MYSQL *connectionPool::getConnection()
{
    MYSQL *conn = nullptr;
    if (connList.size() == 0)
    {
        return nullptr;
    }
    reserve.wait();
    lock.lock();
    conn = connList.front();
    connList.pop_front();
    --freeConnCount;
    ++curConnCount;
    lock.unlock();
    return conn;
}

bool connectionPool::releaseConnection(MYSQL *conn)
{
    if (conn == nullptr)
    {
        return false;
    }
    lock.lock();
    connList.push_back(conn);
    ++freeConnCount;
    --curConnCount;
    lock.unlock();
    reserve.post();
    return true;
}

void connectionPool::destroyPool()
{
    lock.lock();
    if (connList.size() > 0)
    {
        list<MYSQL *>::iterator it;
        for (it = connList.begin(); it != connList.end(); ++it)
        {
            MYSQL *conn = *it;
            mysql_close(conn);
        }
        curConnCount = 0;
        freeConnCount = 0;
        connList.clear();
    }
    lock.unlock();
}

int connectionPool::getFreeConn()
{
    return freeConnCount;
}

connectionPool::~connectionPool()
{
    destroyPool();
}
