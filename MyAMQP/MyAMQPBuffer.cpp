//
//  MyAMQPBuffer.cpp
//  AMQP
//
//  Created by Michael Jones on 19/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyAMQPBuffer.h"
#include <exception>
#include <assert.h>

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
    
    // The Copernica interface is a little strange with its buffer requirements. It is a sort of circular
    // buffer where the amount appended to the end may not be the amount removed from the front.
    void MyAMQPBuffer::AppendBack(std::function<ssize_t(char*, ssize_t)> readFn, ssize_t maxLen) {
        auto storageRequired = _startIndex + _count + maxLen;
        
        if (storageRequired > _buffer.max_size()) {
            // REVIEW: might want to provide a smaller limit.
            throw runtime_error("MyAMQPBuffer: exceeded absolute maximum limit");
        }

        if (maxLen > AvailableForAppend()) {
            _buffer.resize(storageRequired, 0);
        }
        
        assert(maxLen <= AvailableForAppend());
        
        // Dangerous const cast.
        auto bytes = readFn(const_cast<char*>(_buffer.data() + _startIndex + _count), maxLen);
        
         // Update what was actually read into the storage.
        _count += bytes;
    }

    ssize_t MyAMQPBuffer::AvailableForAppend() const {
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