#! /bin/bash

g++ webserver.cpp http_conn.cpp log.cpp -o webserver -pthread