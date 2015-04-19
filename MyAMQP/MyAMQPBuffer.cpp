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
        
        if (maxLen > Available()) {
            throw runtime_error("MyAMQPBuffer: exceeded available");
        }
        
        auto bytes = readFn(const_cast<char*>(_buffer.data()), maxLen);
        
        _count += bytes;
    }

    ssize_t MyAMQPBuffer::Available() const {
        return _buffer.size() - (_startIndex + _count);
    }
    
    ssize_t MyAMQPBuffer::Count() const {
        return _count;
    }
    
    void MyAMQPBuffer::ConsumeFront(ssize_t bytes) {
        if (bytes > Available()) {
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