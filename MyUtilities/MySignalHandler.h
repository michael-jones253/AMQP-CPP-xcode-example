//
//  MySignalHandler.h
//  AMQP
//
//  Created by Michael Jones on 29/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MySignalHandler__
#define __AMQP__MySignalHandler__

#include <stdio.h>

// NB no system signal includes here.
#include <functional>
#include <unordered_map>
#include <memory>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyUtilities {
    using SignalCallback = std::function<void(bool isSelf, bool isDaemon)>;
    
    class MySignalHandler final {
        int _processGroupId;
        
        int _processId;
        
        std::unordered_map<int, std::weak_ptr<SignalCallback const>> _handlers;
        
        // Singletons are usually BAD, but we don't have much choice with signal handling.
        static std::unique_ptr<MySignalHandler> Singleton;
        
    public:
        ~MySignalHandler();
        
        static MySignalHandler* Instance();
        
        void Initialise(bool daemonize);
        
        void InstallHupHandler(std::shared_ptr<SignalCallback const> handler);
        
        void InstallPipeHandler(std::shared_ptr<SignalCallback const> handler);
        
        void InstallTermHandler(std::shared_ptr<SignalCallback const> handler);
        
        // Not for application use.
        void Handle(int sig, int signallingProcessId);
        
    private:
        MySignalHandler();
        
        void Daemonise();
        
        void InstallHandler(int sig, std::shared_ptr<SignalCallback const> handler);
        
    };

}

#pragma GCC visibility pop

#endif /* defined(__AMQP__MySignalHandler__) */
