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
    
    {
        auto callbacks = notifier.CreateSharedCallbacks();
        
        {
            auto localMessage = string{"Hello"};
            auto localCountPtr = unique_ptr<int>(new int{99});
            
            auto erroHandler = [&](string const& err) {
                ++errorCount;
                ++*localCountPtr;
                cout << localMessage << " Global: " << err << endl;
            };
            
            callbacks.SubscribeToError(erroHandler);
        }
        
        notifier.NotifyError("hello");
    }
    
    notifier.NotifyError("goodbye");
    assert(errorCount == 1 && "The second notification should not register due to out of scope");
    
    MyCompletionNotifier anotherNotifier;
    auto moreCallbacks = anotherNotifier.CreateSharedCallbacks();
    
    // moreCallbacks = moreCallbacks;
    // MySharedCallbacks xx;

    
    std::cout << "Hello, World!\n";
    return 0;
}
