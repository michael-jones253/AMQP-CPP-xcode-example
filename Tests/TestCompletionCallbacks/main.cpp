//
//  main.cpp
//  TestCompletionCallbacks
//
//  Created by Michael Jones on 28/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyCompletionNotifier.h"
#include <iostream>
#include <assert.h>

using namespace MyAMQP;
using namespace std;

int main(int argc, const char * argv[]) {
    // insert code here...
    
    MyCompletionNotifier notifier{};
    
    int errorCount{};
    
    auto globalCountPtr = make_shared<int>(0);
    {
        auto callbacks = notifier.CreateCompletionCallbacks();
        auto localMessage = string{"Hello"};
        
        // Just testing scope - this would obviously be risky coding in an application to provide a
        // lambda with a narrower scope than the callbacks that use it.
        {
            // The callbacks are not fool proof and if we declare something that the callback uses
            // which goes out of scope before the callbacks destruct then there will be a problem e.g.
            // auto localCountPtr = unique_ptr<int>(new int{99});
            
            auto localCountPtr = globalCountPtr;
            
            auto erroHandler = [&](string const& err) {
                ++errorCount;
                ++*localCountPtr;
                cout << localMessage << " Global: " << err << endl;
            };
            
            callbacks.SubscribeToError(erroHandler);
        }
        
        // Callbacks still in scope.
        notifier.NotifyError("hello");
    }
    
    // callbacks should have destructed now, so further notifications should have no effect.
    notifier.NotifyError("goodbye");
    
    assert(errorCount == 1 && "The second notification should not register due to out of scope");
    assert(*globalCountPtr == 1 && "Expected shared pointer to have held count");
    
    MyCompletionNotifier anotherNotifier;
    auto moreCallbacks = anotherNotifier.CreateCompletionCallbacks();
    
    // Prevent the following from compiling.
    // moreCallbacks = moreCallbacks;
    // MySharedCallbacks xx;

    
    std::cout << "Hello, World!\n";
    return 0;
}
