#include "config.h"
#include <unistd.h>
#include <stdlib.h>
Config::Config()
{

    // 默认端口为16666
    port = 16666;
    // 默认同步
    is_sync_log = true;
    // LT
    listen_is_LT = true;
    // LT
    conn_is_LT = true;
    // 默认不优雅
    is_opt_linger = false;
    //
    conn_count = 8;
    //
    thread_count = 8;
    // 默认不关闭
    is_close_log = false;
    // 默认是proactor 0是proactor 1是reactor
    actor_model = 0;
}

void Config::parse_args(int argc, char *argv[])
{
    int opt;
    const char *str = "pt:sl:lt:ct:cc:tc:am:ol:cl";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'pt':
        {
            port = atoi(optarg);
            break;
        }
        case 'sl':
        {
            is_sync_log = atoi(optarg);
            break;
        }
        case 'lt':
        {
            listen_is_LT = atoi(optarg);
            break;
        }
        case 'ct':
        {
            conn_is_LT = atoi(optarg);
            break;
        }
        case 'cc':
        {
            conn_count = atoi(optarg);
            break;
        }
        case 'tc':
        {
            thread_count = atoi(optarg);
            break;
        }
        case 'am':
        {
            actor_model = atoi(optarg);
            break;
        }
        case 'ol':
        {
            is_opt_linger = atoi(optarg);
            break;
        }
        case 'cl':
        {
            is_close_log = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}