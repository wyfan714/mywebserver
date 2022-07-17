#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "./threadpool.h"
#include "./http/http_conn.h"
#include "./timer/time_wheel.h"

#define MAX_FD 65536           // 最大文件描述符
#define MAX_EVENT_NUMBER 10000 // 最大事件数
#define TIMESLOT 10            // 最小超时单位

class WebServer
{
public:
    WebServer();
    ~WebServer();
    void init(int port, string user_name, string password, string db_name,
              bool is_sync_log, bool is_opt_linger, bool listen_is_LT, bool conn_is_LT, int conn_count,
              int thread_count, bool is_close_log, int actor_model);

    void thread_pool();
    void connection_pool();
    void init_log();
    void event_listen();
    void serve();
    void add_timer(int connfd, struct sockaddr_in client_address);
    void adjust_timer(tw_timer *timer);
    void del_timer(tw_timer *timer, int sockfd);
    bool deal_client_connect();
    bool deal_with_signal(bool &timeout, bool &stop_server);
    void deal_with_read(int sockfd);
    void deal_with_write(int sockfd);

private:
    int port;
    char *root_dir;
    bool is_sync_log;
    bool is_close_log;
    int actor_model;

    http_conn *users;

    connectionPool *conn_pool;
    string user_name;
    string password;
    string db_name;
    int conn_count;

    threadpool<http_conn> *thd_pool;
    int thread_count;

    epoll_event events[MAX_EVENT_NUMBER];

    int listenfd;
    bool is_opt_linger;
    bool listen_is_LT;
    bool conn_is_LT;
    client_data *users_timer;
    // 定时器相关参数
};
#endif