//
//  MyReceiveTaskQueue.h
//  AMQP
//
//  Created by Michael Jones on 22/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyReceiveTaskQueue__
#define __AMQP__MyReceiveTaskQueue__

#include <stdio.h>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    
    // Generic thread safe queue for movable types.
    template <typename T>
    class MyReceiveTaskQueue final {
        std::mutex _mutex;
        std::condition_variable _conditional;
        std::atomic<bool> _breakWait;
        
        std::queue<T> _itemQueue;
    public:
        MyReceiveTaskQueue();
        
        void BreakWait();
        
        void Reset();
        
        void Push(T&& item);
        
        bool Wait(T& item);
    };
}


#pragma GCC visibility pop
#endif /* defined(__AMQP__MyReceiveTaskQueue__) */
