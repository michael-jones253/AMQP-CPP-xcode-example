//
//  MyAMQPBuffer.h
//  AMQP
//
//  Created by Michael Jones on 19/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyAMQPBuffer__
#define __AMQP__MyAMQPBuffer__

#include <stdio.h>
#include <vector>
#include <functional>

/* The classes below are exported */
#pragma GCC visibility push(default)


namespace MyAMQP {
    class MyAMQPBuffer {
        std::vector<char> _buffer;
        int _startIndex;
        ssize_t _count;
        
    public:
        MyAMQPBuffer();
        
        char const* Get() const;
        
        void AppendBack(std::function<ssize_t(char*, ssize_t)> readFn, ssize_t maxLen);
        
        // The amount of un-consumed data that has been appended.
        ssize_t Count() const;
        
        // The amount available for further data.
        ssize_t Available() const;
        
        // Consume from the front of the data.
        void ConsumeFront(ssize_t bytes);
    };
}
#pragma GCC visibility pop
#endif /* defined(__AMQP__MyAMQPBuffer__) */


