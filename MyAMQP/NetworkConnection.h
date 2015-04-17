//
//  NetworkConnection.h
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef AMQP_NetworkConnection_h
#define AMQP_NetworkConnection_h
#include <functional>
#include <string>

/* The classes below are exported */
#pragma GCC visibility push(default)


class NetworkConnection {
public:
    NetworkConnection(std::function<int(char* buf, int len)> onBytes, std::function<void(std::string const& errString)> onError);
    
    void Open();
    
    void Close();
    
    virtual void Connect(std::string const& ipAddress, int port) = 0;
    
    virtual void Disconnect() = 0;
    
//private:
    virtual int Read(char* buf, int len) = 0;
};

#pragma GCC visibility pop

#endif
