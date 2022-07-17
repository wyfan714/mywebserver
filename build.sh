#! /bin/bash

g++ main.cpp mysql/sql_connection_pool.cpp webserver.cpp config/config.cpp http/http_conn.cpp log/log.cpp -o webserver -pthread -lmysqlclient