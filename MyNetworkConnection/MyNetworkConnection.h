/*
 *  MyNetworkConnection.h
 *  MyNetworkConnection
 *
 *  Created by Michael Jones on 21/04/2015.
 *  Copyright (c) 2015 Michael Jones. All rights reserved.
 *
 */

#ifndef MyNetworkConnection_
#define MyNetworkConnection_

#include "MyAMQPNetworkConnection.h"

#include <stdio.h>
#include <atomic>

/* The classes below are exported */
#pragma GCC visibility push(default)

class MyNetworkConnection final : public MyAMQP::MyAMQPNetworkConnection {
    int _socketFd;
    std::atomic<bool> _canRead;
    
public:
    
    MyNetworkConnection();
    
private:
    // For use by super class only.
    
    void Connect(std::string const& ipAddress, int port) override;
    
    void Disconnect() override;
    
    ssize_t Read(char* buf, size_t len) override;
    
    void WriteAll(char const* buf, size_t len) override;
    
};

#pragma GCC visibility pop
#endif
