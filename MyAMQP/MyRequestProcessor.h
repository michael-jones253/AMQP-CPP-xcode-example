//
//  MyRequestProcessor.h
//  AMQP
//
//  Created by Michael Jones on 5/05/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyRequestProcessor__
#define __AMQP__MyRequestProcessor__

#include "MyReceiveTaskQueue.h"
#include <stdio.h>
#include <future>
#include <atomic>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    class MyRequestProcessor final {
        MyReceiveTaskQueue<std::packaged_task<ssize_t(void)>> _taskQueue;
        std::future<int> _loopHandle;
        
        std::atomic<bool> _shouldRun;
        
    public:
        MyRequestProcessor();
        
        ~MyRequestProcessor();
        
        void Start();
        void Stop();
        
        bool Push(std::packaged_task<ssize_t(void)>&& task);
        
    private:
        int ProcessLoop();
        
        void Flush();
    };
}

#pragma GCC visibility pop


#endif /* defined(__AMQP__MyRequestProcessor__) */
