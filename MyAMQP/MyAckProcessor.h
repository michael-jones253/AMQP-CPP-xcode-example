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
        MyReceiveTaskQueue<std::future<int64_t>> _taskQueue;
        
        std::future<int> _loopHandle;
        
        std::function<void(int64_t)> _ackHandler;

        std::atomic<bool> _shouldRun;
        std::atomic<bool> _flushRequested;
    public:
        MyAckProcessor();
        
        ~MyAckProcessor();
        
        void Start(std::function<void(int64_t)> const& ackHandler);
        void Stop(bool flush);
        void Resume();
        
        void Push(std::future<int64_t>&& deliveryTagFuture);
        
    private:
        int ProcessLoop();
        
        void Flush();
    };
}

#endif /* defined(__AMQP__MyAckProcessor__) */
