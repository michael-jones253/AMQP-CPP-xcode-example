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

// Forward declaration.
struct __siginfo;

namespace MyUtilities {
    using SignalCallback = std::function<void(bool isSelf, bool isDaemon)>;
    
    class MySignalHandler final {
        friend class MySignalCallbacks;
        
        friend void SignalHandler(int sig, __siginfo *siginfo, void *context);
        
        int _processGroupId;
        
        int _processId;
        
        std::unordered_map<int, std::weak_ptr<SignalCallback const>> _handlers;
        
        // Singletons are usually BAD, but we don't have much choice with signal handling.
        static std::unique_ptr<MySignalHandler> Singleton;
        
    public:
        ~MySignalHandler();
        
        static MySignalHandler* Instance();
        
        static bool IsInitialised();
        
        void Initialise(bool daemonize);
        
        
    private:
        MySignalHandler();

        static void Reset();
        
        void Daemonise();
        
        void InstallReloadHandler(std::shared_ptr<SignalCallback const> handler);
        
        void InstallBrokenPipeHandler(std::shared_ptr<SignalCallback const> handler);
        
        void InstallTerminateHandler(std::shared_ptr<SignalCallback const> handler);

        void InstallCtrlCHandler(std::shared_ptr<SignalCallback const> handler);
        
        void InstallHandler(int sig, std::shared_ptr<SignalCallback const> handler);

        // Not for application use.
        void Handle(int sig, int signallingProcessId);
        
    };

}

#pragma GCC visibility pop

#endif /* defined(__AMQP__MySignalHandler__) */
