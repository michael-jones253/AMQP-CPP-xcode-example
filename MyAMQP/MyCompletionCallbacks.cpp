//
//  MyCompletionCallbacks.cpp
//  AMQP
//
//  Created by Michael Jones on 28/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyCompletionCallbacks.h"

using namespace std;
namespace MyAMQP {    
    
    void MyCompletionCallbacks::SubscribeToExit(ExitHandler const& handler) {
        _exitHandler = make_shared<ExitHandler const>(handler);
        
        // Give the notifier temporary ownership of our handler.
        _weakCallbacks->_exitHandler = _exitHandler;
    }
    
    void MyCompletionCallbacks::SubscribeToError(ErrorHandler const& handler) {
        _errorHandler = make_shared<ErrorHandler const>(handler);
        
        // Give the notifier temporary ownership of our handler.
        _weakCallbacks->_errorHandler = _errorHandler;
    }
    
    MyCompletionCallbacks::MyCompletionCallbacks(shared_ptr<MyWeakCallbacks> weakCallbacks) :
    _weakCallbacks{weakCallbacks},
    _exitHandler{},
    _errorHandler{} {
        
    }
}