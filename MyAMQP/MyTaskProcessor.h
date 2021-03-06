//
//  MyTaskProcessor.h
//  AMQP
//
//  Created by Michael Jones on 23/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyTaskProcessor__
#define __AMQP__MyTaskProcessor__

#include "MyReceiveTaskQueue.h"
#include <stdio.h>
#include <future>
#include <atomic>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    class MyTaskProcessor final {
        MyReceiveTaskQueue<std::packaged_task<int64_t(void)>> _taskQueue;        
        std::future<int> _loopHandle;
        
        std::atomic<bool> _shouldRun;
        std::atomic<bool> _flushRequested;
        
    public:
        MyTaskProcessor();
        
        ~MyTaskProcessor();
        
        void Start();
        void Stop(bool flush);
        
        bool Push(std::packaged_task<int64_t(void)>&& task);
        
    private:
        int ProcessLoop();
        
        void Flush();
    };
}

#pragma GCC visibility pop

#endif /* defined(__AMQP__MyTaskProcessor__) */
