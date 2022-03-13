#! /bin/bash

g++ webserver.cpp config/config.cpp http/http_conn.cpp log/log.cpp -o webserver -pthread