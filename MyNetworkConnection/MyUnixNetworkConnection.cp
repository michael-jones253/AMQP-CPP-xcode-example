//
//  MyUnixNetworkConnection.cp
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyUnixNetworkConnection.h"
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace MyAMQP;
using namespace std;

MyUnixNetworkConnection::MyUnixNetworkConnection() :
MyNetworkConnection{},
_socketFd{-1},
_canRead{} {
}

void MyUnixNetworkConnection::Connect(string const& ipAddress, int port) {
    cout << "connecting" << endl;
    
    // Setup socket and connect it.
    _socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketFd < 0)
    {
        string err = "sock you";
        throw runtime_error(err);
    }
    
    struct sockaddr_in sin_you;
    bzero(&sin_you, sizeof(sin_you));
    
    sin_you.sin_family = AF_INET;
    sin_you.sin_port = htons(5672);
    string yourInet = "127.0.0.1";
    auto ret = inet_aton(yourInet.c_str(), &sin_you.sin_addr);
    
    if (ret < 0)
    {
        string err = "you aton";
        throw runtime_error(err);
    }
    
    // Connect to you on send socket.
    int con_ret = connect(_socketFd, (sockaddr*)&sin_you, sizeof(sin_you));
    if (con_ret < 0)
    {
        perror(nullptr);
        string err = "connect you";
        throw runtime_error(err);
    }
    
    _canRead = true;
}

void MyUnixNetworkConnection::Disconnect() {
    _canRead = false;
    close(_socketFd);
    _socketFd = -1;
}

ssize_t MyUnixNetworkConnection::Read(char* buf, size_t len) {
    auto ret = recv(_socketFd, buf, len, 0);
    if (ret < 0) {
        if (_canRead) {
            throw runtime_error("MyUnixNetworkConnection read failed");
        }
        
        // Allow for clean exit if disconnected during read.
        ret = 0;
    }
    
    return ret;
}

void MyUnixNetworkConnection::WriteAll(char const* buf, size_t len) {
    auto amountWritten = len;
    
    while (amountWritten > 0) {
        
        auto ret = send(_socketFd, buf, len, 0);
        
        // FIX ME there are errno codes such as EINTR which need handling.
        if (ret < 0) {
            throw runtime_error("MyUnixNetworkConnection write failed");
        }
        
        amountWritten -= ret;
    }
    
}
