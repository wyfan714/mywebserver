#! /bin/bash


g++ broadcast_server.cpp -o websocket -lboost_system -std=c++11 -lpthread -ljsoncpp
