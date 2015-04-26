//
//  MyStopwatch.h
//  AMQP
//
//  Created by Michael Jones on 25/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyStopwatch__
#define __AMQP__MyStopwatch__

#include <stdio.h>
#include <chrono>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyUtilities {
    
    class MyStopwatch final {
        bool _isRunning;
        
        std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;
        
        std::chrono::time_point<std::chrono::high_resolution_clock> _endTime;
        
    public:
        MyStopwatch();
        
        void Start();
        
        void Restart();
        
        void Stop();
        
        bool IsRunning() const {
            return _isRunning;
        }
        
        std::chrono::milliseconds GetElapsedMilliseconds() const;
    };
}

#pragma GCC visibility pop

#endif /* defined(__AMQP__MyStopwatch__) */
