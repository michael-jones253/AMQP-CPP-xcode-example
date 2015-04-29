//
//  MyNetworkException.h
//  AMQP
//
//  Created by Michael Jones on 29/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyNetworkException__
#define __AMQP__MyNetworkException__

#include <stdio.h>
#include <exception>
#include <string>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyUtilities {
    class MyNetworkException final : public std::runtime_error {
    public:
        MyNetworkException(std::string message, bool useSysError = true);
    };
}

#pragma GCC visibility pop

#endif /* defined(__AMQP__MyNetworkException__) */
