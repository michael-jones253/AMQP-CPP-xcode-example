//
//  MyLoginCredentials.h
//  AMQP
//
//  Created by Michael Jones on 21/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef AMQP_MyLoginCredentials_h
#define AMQP_MyLoginCredentials_h

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    struct MyLoginCredentials {
        std::string HostIpAddress;
        
        std::string UserName;
        
        std::string Password;
    };

}

#pragma GCC visibility pop
#endif
