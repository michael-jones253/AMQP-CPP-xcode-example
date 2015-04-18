//
//  MyNetworkConnection.cpp
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyNetworkConnection.h"
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

using namespace std;

MyNetworkConnection::MyNetworkConnection() : _socketFd{} {
}

void MyNetworkConnection::Connect(string const& ipAddress, int port) {
    cout << "connecting" << endl;
    
    Close();
    
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

}

void MyNetworkConnection::Disconnect() {
    
}

ssize_t MyNetworkConnection::Read(char* buf, size_t len) {
    auto ret = recv(_socketFd, buf, len, 0);
    
    cout << "MyNetworkConnection read:" << ret << endl;

    // FIX ME long ret.
    return ret;
}

ssize_t MyNetworkConnection::Write(char const* buf, size_t len) {
    auto ret = send(_socketFd, buf, len, 0);
    cout << "MyNetworkConnection wrote:" << ret << endl;
    return ret;
}

