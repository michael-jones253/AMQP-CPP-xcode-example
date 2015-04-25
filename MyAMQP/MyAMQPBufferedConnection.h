//
//  MyAMQPBufferedConnection.h
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef AMQP_MyAMQPBufferedConnection_h
#define AMQP_MyAMQPBufferedConnection_h

#include "MyAMQPBuffer.h"
#include <functional>
#include <string>
#include <future>
#include <atomic>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    
    // Abstraction to keep OS dependent socket system out of this library.
    class MyAMQPBufferedConnection {
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
        MyAMQPBufferedConnection();
        
        virtual ~MyAMQPBufferedConnection();
        
        void Open(std::string const& ipAddress, std::function<size_t(char const* buf, ssize_t len)> const& onBytes, std::function<void(std::string const& errString)> const & onError);
        
        void Close();
        
        void SendToServer(char const* buf, size_t len) { WriteAll(buf, len); }
        
    private:
        virtual void Connect(std::string const& ipAddress, int port) = 0;
        
        virtual void Disconnect() = 0;
        
        virtual ssize_t Read(char* buf, size_t len) = 0;
        
        virtual void WriteAll(char const* buf, size_t len) = 0;
        
        void ReadLoop();
    };
}
#pragma GCC visibility pop

#endif
