//
//  MyAMQPBuffer.cpp
//  AMQP
//
//  Created by Michael Jones on 19/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyAMQPBuffer.h"
#include <exception>

using namespace std;

namespace MyAMQP {
    
    MyAMQPBuffer::MyAMQPBuffer() :
    _startIndex{},
    _buffer{},
    _count{} {
        
    }
    
    char const * MyAMQPBuffer::Get() const {
        return _buffer.data() + _startIndex;
    }
    
    void MyAMQPBuffer::AppendBack(std::function<ssize_t(char*, ssize_t)> readFn, ssize_t maxLen) {
        auto projectedSize = _startIndex + _count + maxLen;
        
        if (projectedSize > _buffer.max_size()) {
            throw runtime_error("MyAMQPBuffer: exceeded absolute maximum limit");
        }

        if (projectedSize > Available()) {
            _buffer.resize(projectedSize, 0);
        }
        
        auto bytes = readFn(const_cast<char*>(_buffer.data() + _startIndex + _count), maxLen);
        
        _count += bytes;
    }

    ssize_t MyAMQPBuffer::Available() const {
        return _buffer.size() - (_startIndex + _count);
    }
    
    ssize_t MyAMQPBuffer::Count() const {
        return _count;
    }
    
    void MyAMQPBuffer::ConsumeFront(ssize_t bytes) {
        if (bytes > _count) {
            throw runtime_error("MyAMQPBuffer: Consume exceeds available");
        }
        
        _startIndex += bytes;
        _count -= bytes;
        
        if (_count == 0) {
            // All data consumed, no need to retain existing, can overwrite.
            _startIndex = 0;
        }
    }
    
}