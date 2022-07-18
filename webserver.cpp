#include "webserver.h"

extern int addfd(int epollfd, int fd, bool one_shot, bool is_LT);
extern int removefd(int epollfd, int fd);
extern int setnonblocking(int fd);

static time_wheel tw_wheel;
static int pipefd[2];
static int epollfd;

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

static void sig_handler(int sig)
{
    // 保留原来的errno,在函数最后恢复,以保证函数的可重入性
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}
WebServer::WebServer()
{
    users = new http_conn[MAX_FD];
    users_timer = new client_data[MAX_FD];
    root_dir = "/home/wyf/mywebserver/static";
    addsig(SIGPIPE, SIG_IGN);
}

WebServer::~WebServer()
{
    close(epollfd);
    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    delete[] users;
    delete[] users_timer;
    delete thd_pool;
    // 数据库连接池是单例,不用主动删除
}

void WebServer::init(int port, string user_name, string password, string db_name, bool is_sync_log,
                     bool is_opt_linger, bool listen_is_LT, bool conn_is_LT, int conn_count, int thread_count, bool is_close_log, int actor_model)
{
    this->port = port;
    this->user_name = user_name;
    this->password = password;
    this->db_name = db_name;
    this->conn_count = conn_count;
    this->thread_count = thread_count;
    this->is_sync_log = is_sync_log;
    this->is_opt_linger = is_opt_linger;
    this->listen_is_LT = listen_is_LT;
    this->conn_is_LT = conn_is_LT;
    this->is_close_log = is_close_log;
    this->actor_model = actor_model;
}

void WebServer::init_log()
{
    if (!is_close_log)
    {
        //初始化日志
        if (is_sync_log) // 同步日志
            Log::get_instance()->init("./ServerLog", 2000, 800000, 0);
        else // 异步日志
            Log::get_instance()->init("./ServerLog", 2000, 800000, 800);
    }
}

void WebServer::connection_pool()
{
    //初始化数据库连接池
    conn_pool = connectionPool::getInstance();
    conn_pool->init("localhost", user_name, password, db_name, 3306, conn_count);
}

void WebServer::thread_pool()
{
    //线程池
    thd_pool = new threadpool<http_conn>(actor_model, conn_pool, thread_count);
}

void WebServer::event_listen()
{
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    //优雅关闭连接 tag
    if (!is_opt_linger)
    {
        struct linger tmp = {0, 1};
        setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else
    {
        struct linger tmp = {1, 1};
        setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    int flag = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(listenfd, 5);
    assert(ret >= 0);

    epollfd = epoll_create(5);
    assert(epollfd != -1);
    // 监听连接
    addfd(epollfd, listenfd, false, listen_is_LT);
    http_conn::m_epollfd = epollfd;

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    setnonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0], false, false);
    addsig(SIGALRM, sig_handler, false);
    addsig(SIGTERM, sig_handler, false);
    // 定时
    alarm(TIMESLOT);
}

void timer_handler()
{
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

void WebServer::serve()
{
    bool timeout = false;
    bool stop_server = false;

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

            //处理新到的客户连接
            if (sockfd == listenfd)
            {
                bool status = deal_client_connect();
                if (status == false)
                    continue;
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                // 服务器端关闭连接，移除对应的定时器
                tw_timer *timer = users_timer[sockfd].timer;
                del_timer(timer, sockfd);
            }
            //处理信号
            else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
            {
                bool flag = deal_with_signal(timeout, stop_server);
                if (false == flag)
                    LOG_ERROR("%s", "dealclientdata failure");
            }
            //处理客户连接上接收到的数据
            else if (events[i].events & EPOLLIN)
            {
                deal_with_read(sockfd);
            }
            else if (events[i].events & EPOLLOUT)
            {
                deal_with_write(sockfd);
            }
        }
        if (timeout)
        {
            timer_handler();

            LOG_INFO("%s", "timer tick");

            timeout = false;
        }
    }
}

bool WebServer::deal_client_connect()
{
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    if (listen_is_LT)
    {

        int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
        if (connfd < 0)
        {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }
        if (http_conn::m_user_count >= MAX_FD)
        {
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }
        users[connfd].init(connfd, client_address, root_dir, listen_is_LT, conn_is_LT, is_close_log, user_name, password, db_name);
        users[connfd].init_mysql_result(conn_pool);

        add_timer(connfd, client_address);
    }
    else
    {
        while (1)
        {
            int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addrlength);
            if (connfd < 0)
            {
                LOG_ERROR("%s:errno is:%d", "accept error", errno);
                break;
            }
            if (http_conn::m_user_count >= MAX_FD)
            {
                LOG_ERROR("%s", "Internal server busy");
                break;
            }
            users[connfd].init(connfd, client_address, root_dir, listen_is_LT, conn_is_LT, is_close_log, user_name, password, db_name);
            users[connfd].init_mysql_result(conn_pool);
            add_timer(connfd, client_address);
        }
        return false;
    }
    return true;
}

void WebServer::add_timer(int connfd, struct sockaddr_in client_address)
{
    users_timer[connfd].address = client_address;
    users_timer[connfd].sockfd = connfd;
    tw_timer *timer = tw_wheel.add_timer(TIMESLOT * 3);
    timer->user_data = &users_timer[connfd];
    timer->cb_func = cb_func;
    users_timer[connfd].timer = timer;
}

void WebServer::adjust_timer(tw_timer *timer)
{
    timer->rotation = timer->rotation + 1;

    LOG_INFO("%s", "adjust timer once");
}

void WebServer::del_timer(tw_timer *timer, int sockfd)
{
    timer->cb_func(&users_timer[sockfd]);
    if (timer)
    {
        tw_wheel.del_timer(timer);
    }

    LOG_INFO("close fd %d", users_timer[sockfd].sockfd);
}

bool WebServer::deal_with_signal(bool &timeout, bool &stop_server)
{
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv(pipefd[0], signals, sizeof(signals), 0);
    if (ret == -1)
    {
        return false;
    }
    else if (ret == 0)
    {
        return false;
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
                break;
            }
            }
        }
    }
    return true;
}

void WebServer::deal_with_read(int sockfd)
{
    tw_timer *timer = users_timer[sockfd].timer;

    // reactor
    if (actor_model == 1)
    {
        if (timer)
        {
            adjust_timer(timer);
        }

        //若监测到读事件，将该事件放入请求队列
        thd_pool->append_with_state(users + sockfd, false);

        while (true)
        {
            if (users[sockfd].is_done)
            {
                if (users[sockfd].timer_flag)
                {
                    del_timer(timer, sockfd);
                    users[sockfd].timer_flag = false;
                }
                users[sockfd].is_done = false;
                break;
            }
        }
    }
    else
    {

        // proactor
        if (users[sockfd].read())
        {
            LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));
            //若监测到读事件，将该事件放入请求队列
            thd_pool->append(users + sockfd);

            if (timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            del_timer(timer, sockfd);
        }
    }
}

void WebServer::deal_with_write(int sockfd)
{
    tw_timer *timer = users_timer[sockfd].timer;
    // reactor
    if (actor_model == 1)
    {
        if (timer)
        {
            adjust_timer(timer);
        }

        thd_pool->append_with_state(users + sockfd, true);

        while (true)
        {
            if (users[sockfd].is_done)
            {
                if (users[sockfd].timer_flag)
                {
                    del_timer(timer, sockfd);
                    users[sockfd].timer_flag = false;
                }
                users[sockfd].is_done = false;
                break;
            }
        }
    }
    else
    {
        // proactor
        if (users[sockfd].write())
        {
            LOG_INFO("send data to the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

            if (timer)
            {
                adjust_timer(timer);
            }
        }
        else
        {
            del_timer(timer, sockfd);
        }
    }
}
