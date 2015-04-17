//
//  MyNetworkConnection.h
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyNetworkConnection__
#define __AMQP__MyNetworkConnection__

#include "NetworkConnection.h"

#include <stdio.h>

/* The classes below are exported */
#pragma GCC visibility push(default)

class MyNetworkConnection : public NetworkConnection {
public:
    
    MyNetworkConnection(std::function<int(char* buf, int len)> onBytes, std::function<void(std::string const & errString)> onError);
    
    void Connect(std::string const& ipAddress, int port) override;
    
    void Disconnect() override;
    
    int Read(char* buf, int len) override;
    
};

#pragma GCC visibility pop

#endif /* defined(__AMQP__MyNetworkConnection__) */
