//
//  MyNetworkConnection.h
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyNetworkConnection__
#define __AMQP__MyNetworkConnection__

#include "MyAMQPNetworkConnection.h"

#include <stdio.h>

/* The classes below are exported */
#pragma GCC visibility push(default)

class MyNetworkConnection final : public MyAMQP::MyAMQPNetworkConnection {
    int _socketFd;
    
public:
    
    MyNetworkConnection();
    
private:
    // For use by super class only.
    
    void Connect(std::string const& ipAddress, int port) override;
    
    void Disconnect() override;
    
    ssize_t Read(char* buf, size_t len) override;
    
    ssize_t Write(char const* buf, size_t len) override;
    
};

#pragma GCC visibility pop

#endif /* defined(__AMQP__MyNetworkConnection__) */
