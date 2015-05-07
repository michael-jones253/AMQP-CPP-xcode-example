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
#include "MyRequestResult.h"
#include <stdio.h>
#include <future>
#include <atomic>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    
    // The request processor is for serialising all requests to the Copernica library that are from different
    // thread contexts. This is because the Copernica library is not thread safe.
    class MyRequestProcessor final {
        MyReceiveTaskQueue<std::packaged_task<MyRequestResult(void)>> _taskQueue;
        std::future<int> _loopHandle;
        
        std::atomic<bool> _shouldRun;
        
    public:
        MyRequestProcessor();
        
        ~MyRequestProcessor();
        
        void Start();
        void Stop();
        
        bool Push(std::packaged_task<MyRequestResult(void)>&& task);
        
    private:
        int ProcessLoop();
        
        void Flush();
    };
}

#pragma GCC visibility pop


#endif /* defined(__AMQP__MyRequestProcessor__) */
