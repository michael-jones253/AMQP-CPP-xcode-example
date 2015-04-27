//
//  MyCompletionCallbacks.h
//  AMQP
//
//  Created by Michael Jones on 28/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyCompletionCallbacks__
#define __AMQP__MyCompletionCallbacks__

#include <stdio.h>
#include <functional>
#include <memory>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    
    using ExitHandler = std::function<void(int)>;
    
    using ErrorHandler = std::function<void(std::string const&)>;
    
    struct MyWeakCallbacks {
        std::weak_ptr<ExitHandler const> _exitHandler;
        
        std::weak_ptr<ErrorHandler const> _errorHandler;
    };
    
    class MyCompletionCallbacks final {
        // Private constructor. Only the notifier can create this class.
        friend class MyCompletionNotifier;
        
        std::shared_ptr<MyWeakCallbacks> _weakCallbacks;
        
        std::shared_ptr<ExitHandler const> _exitHandler;
        
        std::shared_ptr<ErrorHandler const> _errorHandler;
        
    public:
        
        void SubscribeToExit(ExitHandler const& handler);
        void SubscribeToError(ErrorHandler const& handler);
        
        MyCompletionCallbacks& operator=(MyCompletionCallbacks const&) = delete;
    private:
        MyCompletionCallbacks(std::shared_ptr<MyWeakCallbacks> weakCallbacks);
    };
}
    
#pragma GCC visibility pop
    
#endif /* defined(__AMQP__MyCompletionCallbacks__) */
