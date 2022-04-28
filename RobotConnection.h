#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#ifdef _WIN32
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

/* For MinGW */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#include <thread>

class RobotConnection {
public:
	RobotConnection(const char* ip_addr, int port);
    bool read_bool(int address);
    void write_bool(int address, bool value);

    int read_register(int address);
    void write_register(int address, int value);

    bool IsConnected();
    void start_server();

    ~RobotConnection();
private:
    static void close_sigint(int dummy);

    void boot_server();
    
    bool validate_address(const int address);
    const char* ip_addr;
    int port;
    int s;

    std::thread th;

    bool kill_thread;
    bool is_connected;

    static modbus_t* ctx;
    static modbus_mapping_t* mb_mapping;

    static int server_socket;
};