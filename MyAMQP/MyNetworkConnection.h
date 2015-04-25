//
//  MyNetworkConnection.h
//  AMQP
//
//  Created by Michael Jones on 25/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef AMQP_MyNetworkConnection_h
#define AMQP_MyNetworkConnection_h

#include <string>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    
    // Abstraction to keep OS dependent socket system out of this library.
    class MyNetworkConnection {
    public:
        virtual ~MyNetworkConnection() {};
        
        virtual void Connect(std::string const& ipAddress, int port) = 0;
        
        virtual void Disconnect() = 0;
        
        virtual ssize_t Read(char* buf, size_t len) = 0;
        
        virtual void WriteAll(char const* buf, size_t len) = 0;
        
        void ReadLoop();
    };
}

#pragma GCC visibility pop

#endif
