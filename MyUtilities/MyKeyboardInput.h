//
//  MyKeyboardInput.h
//  AMQP
//
//  Created by Michael Jones on 1/05/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyKeyboardInput__
#define __AMQP__MyKeyboardInput__

#include <stdio.h>
#include <vector>
#include <string>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyUtilities {
    class MyKeyboardInput final {
    public:
        static char GetOption(std::vector<std::string> options);
    };
    
}

#pragma GCC visibility pop

#endif /* defined(__AMQP__MyKeyboardInput__) */
