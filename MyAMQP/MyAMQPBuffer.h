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
        
        // Consuming from the front moves the start index forward.
        int _startIndex;
        
        // The count of data from the start index i.e. not the same thing as vector size.
        ssize_t _count;
        
    public:
        MyAMQPBuffer();
        
        // Get the start of the un-consumed storage i.e. not the same thing as the start of the
        // underlying storage.
        char const* Get() const;
        
        void AppendBack(std::function<ssize_t(char*, ssize_t)>const& readFn, ssize_t maxLen);
        
        // The amount of un-consumed data that has been appended.
        ssize_t Count() const;
        
        // Consume from the front of the data.
        void ConsumeFront(ssize_t bytes);
        
        // Debug method.
        ssize_t Buffered() const { return _buffer.size(); }

    private:
        // The amount available for further data.
        ssize_t AvailableForAppend() const;
    };
}
#pragma GCC visibility pop
#endif /* defined(__AMQP__MyAMQPBuffer__) */


