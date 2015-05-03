//
//  MyUnixNetworkConnection.cp
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyUnixNetworkConnection.h"
#include "MyNetworkException.h"
#include <algorithm>
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace MyAMQP;
using namespace MyUtilities;
using namespace std;

struct MyUnixNetworkConnectionImpl {
    int SocketFd;
    int BreakFds[2];
    std::atomic<bool> CanRead;
    fd_set ReadSet;
    int FdCount;
};


MyUnixNetworkConnection::MyUnixNetworkConnection() :
MyNetworkConnection{},
_impl{} {
    _impl = unique_ptr<MyUnixNetworkConnectionImpl>(new MyUnixNetworkConnectionImpl);
}

// Necessary to get the impl to link.
MyUnixNetworkConnection::~MyUnixNetworkConnection() {
}

void MyUnixNetworkConnection::Connect(string const& ipAddress, int port) {
    cout << "connecting" << endl;
    
    auto retPipe = pipe(_impl->BreakFds);
    if (retPipe < 0) {
        throw MyNetworkException("Unable to create pipe for break read");
    }
    
    // Setup socket and connect it.
    _impl->SocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_impl->SocketFd < 0)
    {
        string err = "Socket for: " + ipAddress;
        throw MyNetworkException(err);
    }
    
    struct sockaddr_in sinServer;
    bzero(&sinServer, sizeof(sinServer));
    
    sinServer.sin_family = AF_INET;
    sinServer.sin_port = htons(5672);
    string serverInet = "127.0.0.1";
    auto ret = inet_aton(serverInet.c_str(), &sinServer.sin_addr);
    
    if (ret < 0)
    {
        string err = serverInet;
        throw MyNetworkException(err);
    }
    
    // Connect to you on send socket.
    int con_ret = connect(_impl->SocketFd, (sockaddr*)&sinServer, sizeof(sinServer));
    if (con_ret < 0)
    {
        string err = "connect: " + ipAddress;
        throw MyNetworkException(err);
    }
    
    _impl->CanRead = true;
}

void MyUnixNetworkConnection::Disconnect() {
    _impl->CanRead = false;
    close(_impl->SocketFd);
    _impl->SocketFd = -1;
}

ssize_t MyUnixNetworkConnection::Read(char* buf, size_t len) {
    ssize_t readRet{};
    while (_impl->CanRead) {
        FD_ZERO(&_impl->ReadSet);
        FD_SET(_impl->SocketFd, &_impl->ReadSet);
        FD_SET(_impl->BreakFds[0], &_impl->ReadSet);
        
        int numberDescriptors = max(_impl->SocketFd, _impl->BreakFds[0]) + 1;
        
        auto selectRet = select(numberDescriptors, &_impl->ReadSet, nullptr, nullptr, nullptr);
        if (selectRet < 0 && _impl->CanRead) {
            throw MyNetworkException("MyUnixNetworkConnection select failed");
        }
        
        if (FD_ISSET(_impl->SocketFd, &_impl->ReadSet) != 0) {
            
            readRet = recv(_impl->SocketFd, buf, len, 0);
            if (readRet <= 0) {
                auto reason = readRet == 0 ? string{": connection closed by host"} : "";
                auto errStr = "MyUnixNetworkConnection read failed" + reason;
                auto useSysError = readRet != 0;
                
                if (_impl->CanRead) {
                    throw MyNetworkException(errStr, useSysError);
                }
                
                // Allow for clean exit if disconnected during read.
                readRet = 0;
            }
            
            break;
        }
    }
    
    return readRet;
}

void MyUnixNetworkConnection::WriteAll(char const* buf, size_t len) {
    auto amountWritten = len;
    
    while (amountWritten > 0) {
        
        auto ret = send(_impl->SocketFd, buf, len, 0);
        
        // FIX ME there are errno codes such as EINTR which need handling.
        if (ret < 0) {
            auto errStr = "MyUnixNetworkConnection write failed";
            throw MyNetworkException(errStr);
        }
        
        amountWritten -= ret;
    }
    
}

