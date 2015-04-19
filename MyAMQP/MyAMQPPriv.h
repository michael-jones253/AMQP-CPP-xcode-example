/*
 *  MyAMQPPriv.h
 *  MyAMQP
 *
 *  Created by Michael Jones on 17/04/2015.
 *  Copyright (c) 2015 Michael Jones. All rights reserved.
 *
 */

/* The classes below are not exported */
#pragma GCC visibility push(hidden)

namespace MyAMQP {
    
    class MyAMQPPriv
    {
    public:
        void HelloWorldPriv(const char *);
    };
}

#pragma GCC visibility pop
