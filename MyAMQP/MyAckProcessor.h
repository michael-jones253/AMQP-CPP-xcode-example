//
//  MyAckProcessor.h
//  AMQP
//
//  Created by Michael Jones on 23/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyAckProcessor__
#define __AMQP__MyAckProcessor__

#include <stdio.h>
#include "MyReceiveTaskQueue.h"
#include <stdio.h>
#include <future>
#include <atomic>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    class MyAckProcessor final {
        std::atomic<bool> _shouldRun;
        MyReceiveTaskQueue<std::future<int64_t>> _taskQueue;
        
        std::future<int> _loopHandle;
        
    public:
        MyAckProcessor();
        
        ~MyAckProcessor();
        
        void Start();
        void Stop();
        
        void Push(std::future<int64_t>&& deliveryTagFuture);
        
    private:
        int ProcessLoop();
    };
}

#endif /* defined(__AMQP__MyAckProcessor__) */
