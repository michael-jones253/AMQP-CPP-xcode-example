/*
 *  MyUnixNetworkConnection.h
 *  MyUnixNetworkConnection
 *
 *  Created by Michael Jones on 21/04/2015.
 *  Copyright (c) 2015 Michael Jones. All rights reserved.
 *
 */

#ifndef MyUnixNetworkConnection_
#define MyUnixNetworkConnection_

#include "MyNetworkConnection.h"

#include <stdio.h>
#include <atomic>

/* The classes below are exported */
#pragma GCC visibility push(default)

// Forward declaration to hide impl and all OS specific include files.
struct MyUnixNetworkConnectionImpl;

class MyUnixNetworkConnection final : public MyAMQP::MyNetworkConnection {
    std::unique_ptr<MyUnixNetworkConnectionImpl> _impl;
    
public:
    
    MyUnixNetworkConnection();
    ~MyUnixNetworkConnection();
    
private:
    // For use by super class only.
    
    void Connect(std::string const& ipAddress, int port) override;
    
    void Disconnect() override;
    
    ssize_t Read(char* buf, size_t len) override;
    
    void WriteAll(char const* buf, size_t len) override;
    
};

#pragma GCC visibility pop
#endif
