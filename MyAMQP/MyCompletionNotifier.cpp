//
//  MyCompletionNotifier.cpp
//  AMQP
//
//  Created by Michael Jones on 28/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyCompletionNotifier.h"
#include <memory>

using namespace std;
namespace MyAMQP {
    
    
    MyCompletionCallbacks MyCompletionNotifier::CreateCompletionCallbacks() {
        _weakCallbacks = make_shared<MyWeakCallbacks>();
        
        return MyCompletionCallbacks(_weakCallbacks);
    }
    
    void MyCompletionNotifier::NotifyExit(int exitCode) const {
        if (!_weakCallbacks) {
            return;
        }
        
        auto locked = _weakCallbacks->_exitHandler.lock();
        
        if (locked) {
            (*locked)(exitCode);
        }
    }
    
    void MyCompletionNotifier::NotifyError(string const& error) const {
        if (!_weakCallbacks) {
            return;
        }

        auto locked = _weakCallbacks->_errorHandler.lock();
        
        if (locked) {
            (*locked)(error);
        }
    }
}