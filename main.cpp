#include "config/config.h"

int main(int argc, char *argv[])
{
    string user_name = "root";
    string password = "Jintan77";
    string db_name = "webserver";
    Config config;
    config.parse_args(argc, argv);
    WebServer server;
    server.init(config.port, user_name, password, db_name, config.is_sync_log, config.is_opt_linger, config.listen_is_LT, config.conn_is_LT, config.conn_count, config.thread_count, config.is_close_log, config.actor_model);
    // 初始化日志
    server.init_log();
    // 初始化数据库连接池
    server.connection_pool();
    // 初始化线程池
    server.thread_pool();
    // 初始化监听
    server.event_listen();
    // 启动服务
    server.serve();
    return 0;
}
