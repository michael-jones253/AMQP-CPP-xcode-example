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
    int DataSocketFd;
    int ControlFdPair[2];
    std::atomic<bool> DataConnectionOpen;
    std::atomic<bool> ControlUnblockFlagged;
    fd_set ReadSet;
    int FdCount;
};


MyUnixNetworkConnection::MyUnixNetworkConnection() :
MyNetworkConnection{},
_impl{} {
    _impl = make_unique<MyUnixNetworkConnectionImpl>();
}

// Necessary to get the impl to link.
MyUnixNetworkConnection::~MyUnixNetworkConnection() {
}

void MyUnixNetworkConnection::Connect(string const& ipAddress, int port) {
    cout << "connecting" << endl;
    
    // Datagrams will not be lost across AF_UNIX/loopback. We will be nowhere near overflowing the socket buffer.
    // This way we get an immediate send with no delayed buffering.
    auto retPipe = socketpair(AF_UNIX, SOCK_DGRAM, 0, _impl->ControlFdPair);
    if (retPipe < 0) {
        throw MyNetworkException("Unable to create pipe for control connection.");
    }
    
    // Setup socket and connect it.
    _impl->DataSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_impl->DataSocketFd < 0)
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
    int con_ret = connect(_impl->DataSocketFd, (sockaddr*)&sinServer, sizeof(sinServer));
    if (con_ret < 0)
    {
        string err = "connect: " + ipAddress;
        throw MyNetworkException(err);
    }
    
    _impl->DataConnectionOpen = true;
}

void MyUnixNetworkConnection::Disconnect() {
    _impl->DataConnectionOpen = false;
    UnblockRead();
    close(_impl->DataSocketFd);
    close(_impl->ControlFdPair[0]);
}

ssize_t MyUnixNetworkConnection::Read(char* buf, size_t len) {
    ssize_t bytesRead{};
    while (_impl->DataConnectionOpen) {
        FD_ZERO(&_impl->ReadSet);
        FD_SET(_impl->DataSocketFd, &_impl->ReadSet);
        FD_SET(_impl->ControlFdPair[0], &_impl->ReadSet);
        
        int numberDescriptors = max(_impl->DataSocketFd, _impl->ControlFdPair[0]) + 1;
        
        auto selectRet = select(numberDescriptors, &_impl->ReadSet, nullptr, nullptr, nullptr);
        
        if (!_impl->DataConnectionOpen) {
            break;
        }
        
        if (selectRet < 0) {
            throw MyNetworkException("MyUnixNetworkConnection select failed");
        }
        
        // Check for control first and exit select loop here leaving behind any data
        // on the main socket (which will be read when we resume because we do not close the socket on pause.
        if (FD_ISSET(_impl->ControlFdPair[0], &_impl->ReadSet) != 0) {
            char ch{};
            auto nonDataRead = recv(_impl->ControlFdPair[0], &ch, 1, 0);
            if (nonDataRead < 0) {
                throw MyNetworkException("MyUnixNetworkConnection Break FD read error");
            }
            
            if (_impl->ControlUnblockFlagged) {
                break;
            }
            
            // This is normal if the read loop exited because data on the data socket unblocked the read
            // before data on the control socket.
            cerr << "MyUnixNetworkConnection: Spurious control data" << endl;
        }

        if (FD_ISSET(_impl->DataSocketFd, &_impl->ReadSet) != 0) {
            
            bytesRead = recv(_impl->DataSocketFd, buf, len, 0);
            if (bytesRead <= 0) {
                auto reason = bytesRead == 0 ? string{": connection closed by host"} : "";
                auto errStr = "MyUnixNetworkConnection read failed" + reason;
                auto useSysError = bytesRead != 0;
                
                if (_impl->DataConnectionOpen) {
                    throw MyNetworkException(errStr, useSysError);
                }
                
                // Allow for clean exit if disconnected during read.
                bytesRead = 0;
            }
            
            break;
        }
    }
    
    return bytesRead;
}

void MyUnixNetworkConnection::UnblockRead() {
    _impl->ControlUnblockFlagged = true;
    auto bytes = send(_impl->ControlFdPair[1], "u", 1, 0);
    if (bytes < 0) {
        throw MyNetworkException("MyUnixNetworkConnection Break FD write error");
    }
}

void MyUnixNetworkConnection::ResumeRead() {
    _impl->ControlUnblockFlagged = false;
}

void MyUnixNetworkConnection::WriteAll(char const* buf, size_t len) {
    auto amountWritten = len;
    
    while (amountWritten > 0) {
        
        auto ret = send(_impl->DataSocketFd, buf, len, 0);
        
        // FIX ME there are errno codes such as EINTR which need handling.
        if (ret < 0) {
            auto errStr = "MyUnixNetworkConnection write failed";
            throw MyNetworkException(errStr);
        }
        
        amountWritten -= ret;
    }
    
}

