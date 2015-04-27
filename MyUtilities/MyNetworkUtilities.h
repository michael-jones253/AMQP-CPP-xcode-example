//
//  MyNetworkUtilities.h
//  AMQP
//
//  Created by Michael Jones on 27/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyNetworkUtilities__
#define __AMQP__MyNetworkUtilities__

#include <stdio.h>
#include <string>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyUtilities {
    
    class MyNetworkUtilities final {
    public:
        static std::string SysError();
    };
}
#endif /* defined(__AMQP__MyNetworkUtilities__) */
