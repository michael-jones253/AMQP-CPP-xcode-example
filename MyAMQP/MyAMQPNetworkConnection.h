//
//  MyAMQPNetworkConnection.h
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef AMQP_MyAMQPNetworkConnection_h
#define AMQP_MyAMQPNetworkConnection_h

#include "MyAMQPBuffer.h"
#include <functional>
#include <string>
#include <future>
#include <atomic>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    
    class MyAMQPNetworkConnection {
        // Callback for when bytes are available from the network.
        std::function<size_t(char const* buf, ssize_t len)> _onBytes;
        
        // Callback for when a network error is encountered.
        std::function<void(std::string const& errString)> _onError;
        
        // Thread handle.
        std::future<int> _readLoopHandle;
        
        // Whether the read loop should run or not.
        std::atomic<bool> _readShouldRun;
        
        // Special buffer for handling the way the open source library works with buffers and parsing.
        MyAMQPBuffer _amqpBuffer;
        
    public:
        MyAMQPNetworkConnection();
        
        void Open(std::string const& ipAddress, std::function<size_t(char const* buf, ssize_t len)> onBytes, std::function<void(std::string const& errString)> onError);
        
        void Close();
        
        ssize_t SendToServer(char const* buf, size_t len) { return Write(buf, len); }
        
    private:
        virtual void Connect(std::string const& ipAddress, int port) = 0;
        
        virtual void Disconnect() = 0;
        
        virtual ssize_t Read(char* buf, size_t len) = 0;
        
        virtual ssize_t Write(char const* buf, size_t len) = 0;
        
        void ReadLoop();
    };
}
#pragma GCC visibility pop

#endif
