#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

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
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/uio.h>
#include "../log/locker.h"
#include "../mysql/sql_connection_pool.h"
#include "../timer/time_wheel.h"
#include <map>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TBufferTransports.h>
#include "../log/log.h"
#include "../database/crud.h"
#include "../database/skip_types.h"
class http_conn
{
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATCH
    };
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        SUCCESS_REQUEST,
        TEXT_REQUEST,
        FAIL_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    void init(int sockfd, const sockaddr_in &addr, char *root_dir, bool listen_is_LT, bool conn_is_LT, bool is_close_log, string user_name, string password, string db_name, connectionPool *connPool);
    void close_conn(bool read_close = true);
    void process();
    bool read();
    bool write();
    sockaddr_in *get_address()
    {
        return &m_address;
    }

    void init_mysql_result();
    bool timer_flag;
    bool is_done;

private:
    void init();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);

    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE deal_post();
    HTTP_CODE deal_get();
    HTTP_CODE do_request();
    char *get_line() { return m_read_buf + m_start_line; }
    LINE_STATUS parse_line();
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    void add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;
    bool is_write;
    map<string, string> users;

private:
    int m_sockfd;
    sockaddr_in m_address;
    char m_read_buf[READ_BUFFER_SIZE];
    // 已经读入的客户数据的最后一个字节的下一个位置
    int m_read_idx;
    // 当前正在分析的位置
    int m_checked_idx;
    // 当前正在解析的行的起始位置
    int m_start_line;
    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;
    CHECK_STATE m_check_state;
    METHOD m_method;
    char m_real_file[FILENAME_LEN];
    char *m_url;
    char *m_version;
    char *m_host;
    int m_content_length;
    bool m_linger;
    char *m_file_address;
    struct stat m_file_stat;
    struct iovec m_iv[2];
    int m_iv_count;

    char *m_string; //存储请求头数据
    int bytes_to_send;
    int bytes_have_send;
    bool is_post;
    string to_send;
    bool is_close_log;
    bool listen_is_LT;
    bool conn_is_LT;
    char conn_user_name[100];
    char conn_password[100];
    char conn_db_name[100];
    connectionPool *connPool;
    char *root_dir;
    locker map_lock;
};

#endif