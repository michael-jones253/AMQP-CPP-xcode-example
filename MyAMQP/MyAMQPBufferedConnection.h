//
//  MyAMQPBufferedConnection.h
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef AMQP_MyAMQPBufferedConnection_h
#define AMQP_MyAMQPBufferedConnection_h

#include "MyNetworkConnection.h"
#include "MyAMQPBuffer.h"
#include <functional>
#include <string>
#include <future>
#include <atomic>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    using ParseBytesCallback = std::function<size_t(char const* buf, ssize_t len)>;
    using NetworkErrorCallback = std::function<void(std::string const& errString)>;

    
    // Connection to RabbitMQ server with AMQP circular buffer requirements.
    class MyAMQPBufferedConnection final {
        std::unique_ptr<MyNetworkConnection> _networkConnection;
        
        // Callback for when bytes are available from the network.
        ParseBytesCallback _parseReceivedBytes;
        
        // Callback for when a network error is encountered.
        NetworkErrorCallback _onError;
        
        // Thread handle.
        std::future<int> _readLoopHandle;
        
        // Whether the read loop should run or not.
        std::atomic<bool> _readShouldRun;
        
        // Special buffer for handling the way the Copernica open source library works with buffers and parsing.
        MyAMQPBuffer _amqpBuffer;
        
    public:
        
        // C++ Guru Herb Sutter recommends transfer of smart pointer ownership this way.
        MyAMQPBufferedConnection(std::unique_ptr<MyNetworkConnection> networkConnection);
        
        void SetCallbacks(
            ParseBytesCallback const& parseBytesCallback,
            NetworkErrorCallback const & onErrorCallback);
        
        // Final class, virtual not needed.
        ~MyAMQPBufferedConnection();
        
        void Open(std::string const& ipAddress);
        
        void Close();
        
        void SendToServer(char const* buf, size_t len) { WriteAll(buf, len); }
        
    private:
        void Connect(std::string const& ipAddress, int port);
        
        void Disconnect();
        
        ssize_t Read(char* buf, size_t len);
        
        void WriteAll(char const* buf, size_t len);
        
        void ReadLoop();
    };
}
#pragma GCC visibility pop

#endif
