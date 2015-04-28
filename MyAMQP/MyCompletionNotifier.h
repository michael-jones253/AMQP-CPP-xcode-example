//
//  MyCompletionNotifier.h
//  AMQP
//
//  Created by Michael Jones on 28/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyCompletionNotifier__
#define __AMQP__MyCompletionNotifier__

#include <stdio.h>

#include "MyCompletionCallbacks.h"
#include <stdio.h>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {    
    
    class MyCompletionNotifier final {
        // Ownership of callbacks not guaranteed.
        std::shared_ptr<MyWeakCallbacks> _weakCallbacks;
        
    public:
        // Factory method to create callbacks for use by subscribers.
        MyCompletionCallbacks CreateCompletionCallbacks();
        
        void NotifyError(std::string const& error) const;
        void NotifyExit(int exitCode) const;
    };
}

#pragma GCC visibility pop

#endif /* defined(__AMQP__MyCompletionNotifier__) */
