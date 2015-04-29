//
//  MyUnixNetworkConnection.cp
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyUnixNetworkConnection.h"
#include "MyNetworkException.h"
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace MyAMQP;
using namespace MyUtilities;
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
        string err = "Socket for: " + ipAddress;
        throw MyNetworkException(err);
    }
    
    struct sockaddr_in sin_you;
    bzero(&sin_you, sizeof(sin_you));
    
    sin_you.sin_family = AF_INET;
    sin_you.sin_port = htons(5672);
    string yourInet = "127.0.0.1";
    auto ret = inet_aton(yourInet.c_str(), &sin_you.sin_addr);
    
    if (ret < 0)
    {
        string err = yourInet;
        throw MyNetworkException(err);
    }
    
    // Connect to you on send socket.
    int con_ret = connect(_socketFd, (sockaddr*)&sin_you, sizeof(sin_you));
    if (con_ret < 0)
    {
        string err = "connect: " + ipAddress;
        throw MyNetworkException(err);
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
    if (ret <= 0) {
        auto reason = ret == 0 ? string{": connection closed by host"} : "";
        auto errStr = "MyUnixNetworkConnection read failed" + reason;
        auto useSysError = ret != 0;
        
        if (_canRead) {
            throw MyNetworkException(errStr, useSysError);
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
            auto errStr = "MyUnixNetworkConnection write failed";
            throw MyNetworkException(errStr);
        }
        
        amountWritten -= ret;
    }
    
}

