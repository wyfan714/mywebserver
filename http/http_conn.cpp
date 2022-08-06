#include "http_conn.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::database_service;
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file from this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the requested file.\n";
const char *success = "success";
const char *fail = "fail";
int setnonblocking(int fd)
{
    int old_version = fcntl(fd, F_GETFL);
    int new_version = old_version | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_version);
    return old_version;
}

void addfd(int epollfd, int fd, bool one_shot, bool is_LT)
{
    epoll_event event;
    event.data.fd = fd;
    if (!is_LT)
    {
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    }
    else
    {
        event.events = EPOLLIN | EPOLLRDHUP;
    }
    if (one_shot)
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}
void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void modfd(int epollfd, int fd, int ev, bool conn_is_LT)
{
    epoll_event event;
    event.data.fd = fd;
    if (conn_is_LT)
    {
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    }
    else
    {
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    }
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int http_conn::m_user_count = 0;
int http_conn::m_epollfd = -1;
void http_conn::close_conn(bool real_close)
{
    if (real_close && (m_sockfd != -1))
    {
        removefd(m_epollfd, m_sockfd);
        m_sockfd = -1;
        m_user_count--;
    }
}
void http_conn::init(int sockfd, const sockaddr_in &addr, char *root_dir, bool listen_is_LT, bool conn_is_LT, bool is_close_log, string user_name, string password, string db_name, connectionPool *connPool)
{
    m_sockfd = sockfd;
    m_address = addr;
    // debug use
    // int reuse = 1;
    // setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    addfd(m_epollfd, sockfd, true, conn_is_LT);
    m_user_count++;
    this->root_dir = root_dir;
    this->listen_is_LT = listen_is_LT;
    this->conn_is_LT = conn_is_LT;
    this->is_close_log = is_close_log;
    this->connPool = connPool;
    strcpy(conn_user_name, user_name.c_str());
    strcpy(conn_password, password.c_str());
    strcpy(conn_db_name, db_name.c_str());
    init();
}
void http_conn::init()
{
    bytes_to_send = 0;
    bytes_have_send = 0;
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    m_url = 0;
    m_version = 0;
    m_content_length = 0;
    m_host = 0;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    is_post = false;
    is_write = false;
    timer_flag = false;
    is_done = false;
    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file, '\0', FILENAME_LEN);
}

http_conn::LINE_STATUS http_conn::parse_line()
{
    char temp;
    for (; m_checked_idx < m_read_idx; ++m_checked_idx)
    {
        temp = m_read_buf[m_checked_idx];
        if (temp == '\r')
        {
            if ((m_checked_idx + 1) == m_read_idx)
            {
                return LINE_OPEN;
            }
            else if (m_read_buf[m_checked_idx + 1] == '\n')
            {
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if (temp == '\n')
        {
            if ((m_checked_idx > 1) && (m_read_buf[m_checked_idx - 1] == '\r'))
            {
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

bool http_conn::read()
{
    if (m_read_idx >= READ_BUFFER_SIZE)
    {
        return false;
    }
    int bytes_read = 0;
    if (conn_is_LT)
    {
        bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
        m_read_idx += bytes_read;
        if (bytes_read <= 0)
        {
            return false;
        }
        return true;
    }
    else
    {
        while (1)
        {
            bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
            if (bytes_read == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    break;
                }
                return false;
            }
            else if (bytes_read == 0)
            {
                return false;
            }
            m_read_idx += bytes_read;
        }
        return true;
    }
}

http_conn::HTTP_CODE http_conn::parse_request_line(char *text)
{
    // text中是否有 " \t"
    m_url = strpbrk(text, " \t");
    if (!m_url)
    {
        return BAD_REQUEST;
    }
    *m_url++ = '\0';
    char *method = text;
    if (strcasecmp(method, "GET") == 0)
    {
        m_method = GET;
    }
    else if (strcasecmp(method, "POST") == 0)
    {
        m_method = POST;
        is_post = true;
    }
    else
    {
        return BAD_REQUEST;
    }
    // 跳过前面的\t  该函数的意思是找到m_url后面 第一个不在" \t"中的字符
    m_url += strspn(m_url, " \t");
    m_version = strpbrk(m_url, " \t");
    if (!m_version)
    {
        return BAD_REQUEST;
    }
    *m_version++ = '\0';
    m_version += strspn(m_version, " \t");
    if (strcasecmp(m_version, "HTTP/1.1") != 0)
    {
        return BAD_REQUEST;
    }
    if (strncasecmp(m_url, "http://", 7) == 0)
    {
        m_url += 7;
        m_url = strchr(m_url, '/');
    }
    if (strncasecmp(m_url, "https://", 8) == 0)
    {
        m_url += 8;
        m_url = strchr(m_url, '/');
    }
    if (!m_url || m_url[0] != '/')
    {
        return BAD_REQUEST;
    }

    //
    if (strlen(m_url) == 1)
    {
        strcat(m_url, "templates/welcome.html");
    }
    std::cout << m_url << std::endl;
    m_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::parse_headers(char *text)
{
    if (text[0] == '\0')
    {
        if (m_content_length != 0)
        {
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    else if (strncasecmp(text, "Connection:", 11) == 0)
    {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0)
        {
            m_linger = true;
        }
    }
    else if (strncasecmp(text, "Content-Length:", 15) == 0)
    {
        text += 15;
        text += strspn(text, " \t");
        m_content_length = atol(text);
    }
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        m_host = text;
    }
    else
    {
        // printf("oop! unknow head %s\n", text);
        LOG_INFO("oop!unknow header: %s", text);
        Log::get_instance()->flush();
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::parse_content(char *text)
{
    if (m_read_idx >= (m_content_length + m_checked_idx))
    {
        text[m_content_length] = '\0';
        m_string = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

// a litter problem
http_conn::HTTP_CODE http_conn::process_read()
{
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text = 0;
    while (((m_check_state == CHECK_STATE_CONTENT) && (line_status == LINE_OK)) || ((line_status = parse_line()) == LINE_OK))
    {
        text = get_line();
        m_start_line = m_checked_idx;
        // printf("got 1 http line:%s\n", text);
        switch (m_check_state)
        {
        case CHECK_STATE_REQUESTLINE:
        {
            ret = parse_request_line(text);
            if (ret == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            break;
        }
        case CHECK_STATE_HEADER:
        {

            ret = parse_headers(text);
            if (ret == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            else if (ret == GET_REQUEST)
            {
                return do_request();
            }
            break;
        }
        case CHECK_STATE_CONTENT:
        {
            ret = parse_content(text);
            if (ret == GET_REQUEST)
            {
                return do_request();
            }
            line_status = LINE_OPEN;
            break;
        }
        default:
        {
            return INTERNAL_ERROR;
            break;
        }
        }
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::deal_post()
{
    const char *p = strrchr(m_url, '/');
    if (!strcmp(p + 1, "login"))
    {
        //将用户名和密码提取出来
        // user=123&passwd=123
        char name[100], password[100];
        int i;
        for (i = 5; m_string[i] != '&'; ++i)
            name[i - 5] = m_string[i];
        name[i - 5] = '\0';

        int j = 0;
        for (i = i + 8; m_string[i] != '\0'; ++i, ++j)
            password[j] = m_string[i];
        password[j] = '\0';
        init_mysql_result();
        if (users.find(name) != users.end() && users[name] == password)
            return SUCCESS_REQUEST;
        else
            return FAIL_REQUEST;
    }
    else if (!strcmp(p + 1, "register"))
    {
        //将用户名和密码提取出来
        // user=123&passwd=123
        char name[100], password[100];
        int i;
        for (i = 5; m_string[i] != '&'; ++i)
            name[i - 5] = m_string[i];
        name[i - 5] = '\0';

        int j = 0;
        for (i = i + 8; m_string[i] != '\0'; ++i, ++j)
            password[j] = m_string[i];
        password[j] = '\0';
        cout << name << " " << password << endl;
        //如果是注册，先检测数据库中是否有重名的
        //没有重名的，进行增加数据
        char *sql_insert = (char *)malloc(sizeof(char) * 200);
        strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
        strcat(sql_insert, "'");
        strcat(sql_insert, name);
        strcat(sql_insert, "', '");
        strcat(sql_insert, password);
        strcat(sql_insert, "')");
        init_mysql_result();
        if (users.find(name) == users.end())
        {
            int res = mysql_query(mysql, sql_insert);
            if (!res)
            {
                return SUCCESS_REQUEST;
            }
            else
            {
                return FAIL_REQUEST;
            }
        }
        else
            return FAIL_REQUEST;
    }
    else if (!strcmp(p + 1, "search"))
    {
        std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
        std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        crudClient client(protocol);
        try
        {
            transport->open();
            char key[100];
            int i;
            for (i = 4; m_string[i] != '\0'; ++i)
                key[i - 4] = m_string[i];
            key[i - 4] = '\0';
            client.search_element(to_send, key);
            cout << to_send << endl;
            transport->close();
            return TEXT_REQUEST;
        }
        catch (TException &tx)
        {
            cout << "ERROR: " << tx.what() << endl;
        }
    }
    else if (!strcmp(p + 1, "delete"))
    {
        std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
        std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        crudClient client(protocol);
        try
        {
            transport->open();
            char key[100];
            int i;
            for (i = 4; m_string[i] != '\0'; ++i)
                key[i - 4] = m_string[i];
            key[i - 4] = '\0';
            // 1 为成功 0 为失败
            int ret = client.delete_element(key);
            transport->close();
            if (ret == 1)
            {
                return SUCCESS_REQUEST;
            }
            else
            {
                return FAIL_REQUEST;
            }
        }
        catch (TException &tx)
        {
            cout << "ERROR: " << tx.what() << endl;
        }
    }
    else if (!strcmp(p + 1, "insert"))
    {
        std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
        std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        crudClient client(protocol);
        try
        {
            transport->open();
            // key=123&value=123
            char key[100], value[100];
            int i;
            for (i = 4; m_string[i] != '&'; ++i)
                key[i - 4] = m_string[i];
            key[i - 4] = '\0';

            int j = 0;
            for (i = i + 7; m_string[i] != '\0'; ++i, ++j)
                value[j] = m_string[i];
            value[j] = '\0';
            // 1 为成功 0 为失败
            int ret = client.insert_element(key, value);
            transport->close();
            if (ret == 1)
            {
                return SUCCESS_REQUEST;
            }
            else
            {
                return FAIL_REQUEST;
            }
        }
        catch (TException &tx)
        {
            cout << "ERROR: " << tx.what() << endl;
        }
    }
}

http_conn::HTTP_CODE http_conn::deal_get()
{
    strcpy(m_real_file, root_dir);
    int len = strlen(root_dir);
    strncpy(m_real_file + len, m_url, FILENAME_LEN - len - 1);
    if (stat(m_real_file, &m_file_stat) < 0)
    {
        return NO_RESOURCE;
    }
    if (!(m_file_stat.st_mode & S_IROTH))
    {
        return FORBIDDEN_REQUEST;
    }
    if (S_ISDIR(m_file_stat.st_mode))
    {
        return BAD_REQUEST;
    }
    int fd = open(m_real_file, O_RDONLY);
    m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

http_conn::HTTP_CODE http_conn::do_request()
{
    if (is_post)
    {
        return deal_post();
    }
    else
    {
        return deal_get();
    }
}

void http_conn::unmap()
{
    if (m_file_address)
    {
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = 0;
    }
}
bool http_conn::write()
{
    int temp = 0;
    if (bytes_to_send == 0)
    {
        modfd(m_epollfd, m_sockfd, EPOLLIN, conn_is_LT);
        init();
        return true;
    }
    while (1)
    {
        temp = writev(m_sockfd, m_iv, m_iv_count);
        if (temp <= -1)
        {

            if (errno == EAGAIN)
            {
                modfd(m_epollfd, m_sockfd, EPOLLOUT, conn_is_LT);
                return true;
            }
            unmap();
            return false;
        }
        bytes_to_send -= temp;
        bytes_have_send += temp;
        if (bytes_have_send >= m_iv[0].iov_len)
        {
            m_iv[0].iov_len = 0;
            m_iv[1].iov_base = m_file_address + (bytes_have_send - m_write_idx);
            m_iv[1].iov_len = bytes_to_send;
        }
        else
        {
            m_iv[0].iov_base = m_write_buf + bytes_have_send;
            m_iv[0].iov_len = m_iv[0].iov_len - bytes_have_send;
        }

        if (bytes_to_send <= 0)
        {
            unmap();
            modfd(m_epollfd, m_sockfd, EPOLLIN, conn_is_LT);

            if (m_linger)
            {
                init();
                return true;
            }
            else
            {
                return false;
            }
        }
        // if (bytes_to_send <= bytes_have_send)
        // {
        //     unmap();
        //     if (m_linger)
        //     {
        //         init();
        //         modfd(m_epollfd, m_sockfd, EPOLLIN);
        //         return true;
        //     }
        //     else
        //     {
        //         modfd(m_epollfd, m_sockfd, EPOLLIN);
        //         // return false;
        //         return true;
        //     }
        // }
    }
}
// dont clear
bool http_conn::add_response(const char *format, ...)
{
    if (m_write_idx >= WRITE_BUFFER_SIZE)
    {
        return false;
    }
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx))
    {
        return false;
    }
    m_write_idx += len;
    va_end(arg_list);
    LOG_INFO("request:%s", m_write_buf);
    Log::get_instance()->flush();
    return true;
}

bool http_conn::add_status_line(int status, const char *title)
{
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}
// bool -> void
void http_conn::add_headers(int content_len)
{
    add_content_length(content_len);
    add_linger();
    add_blank_line();
}
bool http_conn::add_content_length(int content_len)
{
    return add_response("Content-Length: %d\r\n", content_len);
}
bool http_conn::add_linger()
{
    return add_response("Connection: %s\r\n", (m_linger == true) ? "keep-alive" : "close");
}
bool http_conn::add_blank_line()
{
    return add_response("%s", "\r\n");
}
bool http_conn::add_content(const char *content)
{
    return add_response("%s", content);
}

bool http_conn::process_write(HTTP_CODE ret)
{
    switch (ret)
    {
    case INTERNAL_ERROR:
    {
        add_status_line(500, error_500_title);
        add_headers(strlen(error_500_form));
        if (!add_content(error_500_form))
        {
            return false;
        }
        break;
    }
    case BAD_REQUEST:
    {
        add_status_line(400, error_400_title);
        add_headers(strlen(error_400_form));
        if (!add_content(error_400_form))
        {
            return false;
        }
        break;
    }
    case NO_RESOURCE:
    {
        add_status_line(404, error_404_title);
        add_headers(strlen(error_404_form));
        if (!add_content(error_404_form))
        {
            return false;
        }
        break;
    }
    case FORBIDDEN_REQUEST:
    {
        add_status_line(403, error_403_title);
        add_headers(strlen(error_403_form));
        if (!add_content(error_403_form))
        {
            return false;
        }
        break;
    }
    case FAIL_REQUEST:
    {
        add_status_line(200, ok_200_title);
        add_headers(strlen(fail));
        if (!add_content(fail))
        {
            return false;
        }
        break;
    }
    case SUCCESS_REQUEST:
    {
        add_status_line(200, ok_200_title);
        add_headers(strlen(success));
        if (!add_content(success))
        {
            return false;
        }
        break;
    }
    case TEXT_REQUEST:
    {
        add_status_line(200, ok_200_title);
        add_headers(strlen(to_send.c_str()));
        if (!add_content(to_send.c_str()))
        {
            return false;
        }
        break;
    }
    case FILE_REQUEST:
    {
        add_status_line(200, ok_200_title);
        if (m_file_stat.st_size != 0)
        {
            add_headers(m_file_stat.st_size);
            m_iv[0].iov_base = m_write_buf;
            m_iv[0].iov_len = m_write_idx;
            m_iv[1].iov_base = m_file_address;
            m_iv[1].iov_len = m_file_stat.st_size;
            m_iv_count = 2;
            bytes_to_send = m_write_idx + m_file_stat.st_size;
            return true;
        }
        else
        {
            const char *ok_string = "<html><body></body></html>";
            add_headers(strlen(ok_string));
            if (!add_content(ok_string))
            {
                return false;
            }
        }
        break;
    }
    default:
    {
        return false;
    }
    }
    m_iv[0].iov_base = m_write_buf;
    m_iv[0].iov_len = m_write_idx;
    m_iv_count = 1;
    bytes_to_send = m_write_idx;
    return true;
}

void http_conn::process()
{
    HTTP_CODE read_ret = process_read();
    if (read_ret == NO_REQUEST)
    {
        modfd(m_epollfd, m_sockfd, EPOLLIN, conn_is_LT);
        return;
    }
    bool write_ret = process_write(read_ret);
    if (!write_ret)
    {
        close_conn();
    }
    modfd(m_epollfd, m_sockfd, EPOLLOUT, conn_is_LT);
}

void http_conn::init_mysql_result()
{
    //先从连接池中取一个连接
    MYSQL *mysql = connPool->getConnection();

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }

    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);

    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);

    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    users.clear();
    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        string temp1(row[0]);
        string temp2(row[1]);
        users[temp1] = temp2;
    }
    connPool->releaseConnection(mysql);
}