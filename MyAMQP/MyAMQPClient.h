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
// POD and the only Copernica external library header we expose.
#include "exchangeType.h"
#include "MyAMQPTypes.h"
#include "MyAMQPNetworkConnection.h"
#include "MyLoginCredentials.h"
#include "MyAMQPRoutingInfo.h"

#include <string>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    // Forward declaration to avoid #include of impl in this header file.
    class MyAMQPClientImpl;
    
    class MyAMQPClient final {
        
        std::unique_ptr<MyAMQPClientImpl> _impl;
        
    public:
        void CreateHelloQueue(AMQP::ExchangeType exchangeType, MyAMQPRoutingInfo const& routingInfo);
        
        void SendHelloWorld(std::string const& exchange, std::string const& key, std::string const& greeting);
        
        void SubscribeToReceive(std::string const& queue,
                                MyMessageCallback const &userHandler,
                                bool threaded);
        
        MyAMQPClient(std::unique_ptr<MyAMQPNetworkConnection> networkConnection);

        // Class is final, virtual not needed.
        ~MyAMQPClient();
        
        void Open(MyLoginCredentials const& loginInfo);
        
        void Close();
        
    private:
        size_t OnNetworkRead(char const* buf, int len);
        
        void OnNetworkReadError(std::string const& errorStr);
        
    };
}
#pragma GCC visibility pop
#endif
