#include "config.h"
#include <unistd.h>
#include <stdlib.h>
Config::Config()
{

    // 默认端口为16666
    PORT = 16666;

    LISTENTYPE = 0;

    CONNTYPE = 0;

    LOGTYPE = 0;

    TIMERTYPE = 0;
}

void Config::parse_args(int argc, char *argv[])
{
    int opt;
    const char *str = "p:l:c:g:t";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            PORT = atoi(optarg);
            break;
        }
        case 'l':
        {
            LISTENTYPE = atoi(optarg);
            break;
        }
        case 'c':
        {
            CONNTYPE = atoi(optarg);
            break;
        }
        case 'g':
        {
            LOGTYPE = atoi(optarg);
            break;
        }
        case 't':
        {
            TIMERTYPE = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}