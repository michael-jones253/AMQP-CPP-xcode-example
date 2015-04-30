//
//  MySignalCallbacks.cpp
//  AMQP
//
//  Created by Michael Jones on 30/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MySignalCallbacks.h"
#include <assert.h>

using namespace std;

namespace MyUtilities {
    MySignalCallbacks::MySignalCallbacks() {        
        assert(MySignalHandler::IsInitialised());
    }

    MySignalCallbacks::~MySignalCallbacks() {
        // RAII restoration of default system handlers once this set of handlers is out of scope.
        MySignalHandler::Reset();
    }

    void MySignalCallbacks::InstallTerminateHandler(SignalCallback const& handler) {
        auto sharedHandler = make_shared<SignalCallback const>(handler);
        _terminate = sharedHandler;
        
        MySignalHandler::Instance()->InstallTerminateHandler(sharedHandler);
    }
    
    void MySignalCallbacks::InstallCtrlCHandler(SignalCallback const& handler) {
        auto sharedHandler = make_shared<SignalCallback const>(handler);
        _ctrlC = sharedHandler;
        
        MySignalHandler::Instance()->InstallCtrlCHandler(sharedHandler);
    }
    
    void MySignalCallbacks::InstallReloadHandler(SignalCallback const& handler) {
        auto sharedHandler = make_shared<SignalCallback const>(handler);
        _reload = sharedHandler;
        
        MySignalHandler::Instance()->InstallReloadHandler(sharedHandler);
    }
    
    void MySignalCallbacks::InstallBrokenPipeHandler(SignalCallback const& handler) {
        auto sharedHandler = make_shared<SignalCallback const>(handler);
        _brokenPipe = sharedHandler;
        
        MySignalHandler::Instance()->InstallBrokenPipeHandler(sharedHandler);
    }
    
}
