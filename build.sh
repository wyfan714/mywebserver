#! /bin/bash

g++ webserver.cpp config.cpp http_conn.cpp log.cpp -o webserver -pthread