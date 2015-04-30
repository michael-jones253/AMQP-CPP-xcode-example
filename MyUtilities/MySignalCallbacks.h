//
//  MySignalCallbacks.h
//  AMQP
//
//  Created by Michael Jones on 30/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MySignalCallbacks__
#define __AMQP__MySignalCallbacks__

#include "MySignalHandler.h"
#include <stdio.h>
#include <functional>
#include <memory>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyUtilities {

class MySignalCallbacks {
    std::shared_ptr<SignalCallback const> _terminate;
    std::shared_ptr<SignalCallback const> _ctrlC;
    std::shared_ptr<SignalCallback const> _reload;
    std::shared_ptr<SignalCallback const> _brokenPipe;
    
public:
    void InstallTerminateHandler(SignalCallback const& handler);
    void InstallCtrlCHandler(SignalCallback const& handler);
    void InstallReloadHandler(SignalCallback const& handler);
    void InstallBrokenPipeHandler(SignalCallback const& handler);
    
    MySignalCallbacks();
    
    ~MySignalCallbacks();

};

}

#pragma GCC visibility pop

#endif /* defined(__AMQP__MySignalCallbacks__) */
