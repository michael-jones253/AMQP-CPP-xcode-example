/*
 *  MyAMQP.h
 *  MyAMQP
 *
 *  Created by Michael Jones on 17/04/2015.
 *  Copyright (c) 2015 Michael Jones. All rights reserved.
 *
 */

#ifndef MyAMQP_
#define MyAMQP_
// This is the only Copernica library header we expose - enum only.
#include "exchangeType.h"

// POD and pure virtual abstractions only get exposed here.
#include "MyAMQPTypes.h"
#include "MyNetworkConnection.h"
#include "MyLoginCredentials.h"
#include "MyAMQPRoutingInfo.h"
#include "MyCompletionCallbacks.h"

#include <string>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    // Forward declaration to avoid #include of impl in this header file.
    class MyAMQPClientImpl;
    
    class MyAMQPClient final {
        // Impl to hide implementation headers from application code - speeds up compiling.
        std::unique_ptr<MyAMQPClientImpl> _impl;
        
    public:
        void CreateHelloQueue(AMQP::ExchangeType exchangeType, MyAMQPRoutingInfo const& routingInfo);
        
        void SendHelloWorld(std::string const& exchange, std::string const& key, std::string const& greeting);
        
        void SubscribeToReceive(std::string const& queue,
                                MyMessageCallback const &userHandler,
                                bool threaded);
        
        MyAMQPClient(std::unique_ptr<MyNetworkConnection> networkConnection);

        // Class is final, virtual not needed.
        ~MyAMQPClient();
        
        MyCompletionCallbacks Open(MyLoginCredentials const& loginInfo);
        
        void Close(bool flush);
        
    private:
        size_t OnNetworkRead(char const* buf, int len);
        
        void OnNetworkReadError(std::string const& errorStr);
        
    };
}
#pragma GCC visibility pop
#endif
