#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "log/locker.h"
#include "threadpool.h"
#include "http/http_conn.h"
// #include "timer/lst_timer.h"
#include "timer/time_wheel.h"
#include "log/log.h"
#include "config/config.h"
#define MAX_FD 65536           // 最大文件描述符
#define MAX_EVENT_NUMBER 10000 // 最大事件数
#define TIMESLOT 10            // 最小超时单位

extern int addfd(int epollfd, int fd, bool one_shot, int LISTENTYPE, int CONNTYPE);
extern int removefd(int epollfd, int fd);
extern int setnonblocking(int fd);

// 定时器相关参数
static int pipefd[2]; // 用于接收信号
// static sort_timer_lst timer_lst;
static time_wheel tw_wheel;
static int epollfd = 0;

void sig_handler(int sig)
{
    // 保留原来的errno,在函数最后恢复,以保证函数的可重入性
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void timer_handler()
{
    // timer_lst.tick();
    tw_wheel.tick();
    alarm(TIMESLOT);
}

void cb_func(client_data *user_data)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    http_conn::m_user_count--;
    LOG_INFO("close fd %d", user_data->sockfd);
    Log::get_instance()->flush();
}
void addsig(int sig, void (*handler)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void show_error(int connfd, const char *info)
{
    printf("%s", info);
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int main(int argc, char *argv[])
{
    Config config;
    config.parse_args(argc, argv);
    if (config.LOGTYPE == 0)
    {
        //同步日志模型
        Log::get_instance()->init("ServerLog", 2000, 800000, 0);
    }
    else
    {
        //异步日志模型
        Log::get_instance()->init("ServerLog", 2000, 800000, 800);
    }

    int port = config.PORT;

    // 忽略SITPIPE信号
    addsig(SIGPIPE, SIG_IGN);

    // 创建线程池
    threadpool<http_conn> *pool = NULL;
    try
    {
        pool = new threadpool<http_conn>;
    }
    catch (...)
    {
        return 1;
    }

    // 预先为每个可能的客户连接分配一个http_conn对象
    http_conn *users = new http_conn[MAX_FD];
    assert(users);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    // 有数据待发送, 则延迟关闭
    struct linger tmp = {1, 0};
    setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    // inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int flag = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(listenfd, 5);
    assert(ret >= 0);

    epoll_event events[MAX_EVENT_NUMBER];
    epollfd = epoll_create(5);
    assert(epollfd != -1);
    // 监听连接
    addfd(epollfd, listenfd, false, config.LISTENTYPE, config.CONNTYPE);
    http_conn::m_epollfd = epollfd;

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    setnonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0], false, config.LISTENTYPE, config.CONNTYPE);
    addsig(SIGALRM, sig_handler, false);
    addsig(SIGTERM, sig_handler, false);
    bool stop_server = false;
    client_data *users_timer = new client_data[MAX_FD];
    bool timeout = false;
    // 定时
    alarm(TIMESLOT);

    while (!stop_server)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if ((number < 0) && (errno != EINTR))
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }
        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd)
            {

                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                if (config.LISTENTYPE == 0)
                {
                    // listenfd LT
                    int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
                    if (connfd < 0)
                    {
                        LOG_ERROR("%s:errno is: %d", "accept error", errno);
                        continue;
                    }
                    if (http_conn::m_user_count >= MAX_FD)
                    {
                        show_error(connfd, "Internal server busy");
                        LOG_ERROR("%s", "Internal server busy");
                        continue;
                    }
                    users[connfd].init(connfd, client_address, config.LISTENTYPE, config.CONNTYPE);

                    users_timer[connfd].address = client_address;
                    users_timer[connfd].sockfd = connfd;
                    // util_timer *timer = new util_timer;
                    tw_timer *timer = tw_wheel.add_timer(TIMESLOT * 3);
                    timer->user_data = &users_timer[connfd];
                    timer->cb_func = cb_func;
                    // time_t cur = time(NULL);
                    // timer->expire = cur + 3 * TIMESLOT;
                    users_timer[connfd].timer = timer;
                    // timer_lst.add_timer(timer);
                }
                else
                {
                    // listenfd ET
                    while (1)
                    {
                        int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
                        if (connfd < 0)
                        {
                            LOG_ERROR("%s:errno is: %d", "accept error", errno);
                            continue;
                        }
                        if (http_conn::m_user_count >= MAX_FD)
                        {
                            show_error(connfd, "Internal server busy");
                            LOG_ERROR("%s", "Internal server busy");
                            continue;
                        }
                        users[connfd].init(connfd, client_address, config.LISTENTYPE, config.CONNTYPE);

                        users_timer[connfd].address = client_address;
                        users_timer[connfd].sockfd = connfd;
                        // util_timer *timer = new util_timer;
                        tw_timer *timer = tw_wheel.add_timer(TIMESLOT * 3);
                        timer->user_data = &users_timer[connfd];
                        timer->cb_func = cb_func;
                        // time_t cur = time(NULL);
                        // timer->expire = cur + 3 * TIMESLOT;
                        users_timer[connfd].timer = timer;
                        // timer_lst.add_timer(timer);
                    }
                }
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                // 客户端关闭连接, 移除对应的定时器
                // util_timer *timer = users_timer[sockfd].timer;
                tw_timer *timer = users_timer[sockfd].timer;
                timer->cb_func(&users_timer[sockfd]);
                if (timer)
                {
                    // timer_lst.del_timer(timer);
                    tw_wheel.del_timer(timer);
                }
            }
            else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
            {
                // 处理信号
                int sig;
                char signals[1024];
                ret = recv(pipefd[0], signals, sizeof(signals), 0);
                if (ret == -1)
                {
                    continue;
                }
                else if (ret == 0)
                {
                    continue;
                }
                else
                {
                    for (int i = 0; i < ret; ++i)
                    {
                        switch (signals[i])
                        {
                        case SIGALRM:
                        {
                            timeout = true;
                            break;
                        }
                        case SIGTERM:
                        {
                            stop_server = true;
                        }
                        }
                    }
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                // 处理从客户连接上收到的数据
                // util_timer *timer = users_timer[sockfd].timer;
                tw_timer *timer = users_timer[sockfd].timer;
                if (users[sockfd].read())
                {
                    LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));
                    Log::get_instance()->flush();
                    pool->append(users + sockfd);
                    if (timer)
                    {
                        // time_t cur = time(NULL);
                        // timer->expire = cur + 3 * TIMESLOT;
                        // 时间轮圈数+1
                        timer->rotation = timer->rotation + 1;
                        LOG_INFO("%s", "adjust timer once");
                        Log::get_instance()->flush();
                        // timer_lst.adjust_timer(timer);
                    }
                }
                else
                {
                    timer->cb_func(&users_timer[sockfd]);
                    if (timer)
                    {
                        // timer_lst.del_timer(timer);
                        tw_wheel.del_timer(timer);
                    }
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                // util_timer *timer = users_timer[sockfd].timer;
                tw_timer *timer = users_timer[sockfd].timer;
                if (users[sockfd].write())
                {
                    LOG_INFO("send data to the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));
                    Log::get_instance()->flush();
                    if (timer)
                    {
                        // time_t cur = time(NULL);
                        // timer->expire = cur + 3 * TIMESLOT;
                        // 时间轮圈数+1
                        timer->rotation = timer->rotation + 1;
                        LOG_INFO("%s", "adjust timer once");
                        Log::get_instance()->flush();
                        // timer_lst.adjust_timer(timer);
                    }
                }
                else
                {
                    timer->cb_func(&users_timer[sockfd]);
                    if (timer)
                    {
                        // timer_lst.del_timer(timer);
                        tw_wheel.del_timer(timer);
                    }
                    // users[sockfd].close_conn();
                }
            }
            else
            {
            }
        }
        if (timeout)
        {
            timer_handler();
            timeout = false;
        }
    }
    close(epollfd);
    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    delete[] users;
    delete[] users_timer;
    delete pool;
    return 0;
}
