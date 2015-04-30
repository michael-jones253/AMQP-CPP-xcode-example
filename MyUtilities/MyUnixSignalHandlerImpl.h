//
//  MyUnixSignalHandlerImpl.h
//  AMQP
//
//  Created by Michael Jones on 30/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyUnixSignalHandlerImpl__
#define __AMQP__MyUnixSignalHandlerImpl__

#include <stdio.h>
#include <signal.h>
#include <functional>
#include <set>

#define MY_SIGHUP SIGHUP
#define MY_SIGTERM SIGTERM
#define MY_SIGINT SIGINT
#define MY_SIGPIPE SIGPIPE

namespace MyUtilities {
    using SignalFunction = std::function<void(int sig, bool fromSelf, bool isDaemon)>;
    
    class MyUnixSignalHandlerImpl final {
        friend void UnixSignalHandler(int sig, siginfo_t *siginfo, void *context);
        
        static struct sigaction SigAct;
        
        static SignalFunction SignalCallback;
        
        static int ProcessGroupId;
        
        static int ProcessId;
        
        std::set<int> _handledSignals;
        
    public:
        MyUnixSignalHandlerImpl(SignalFunction const& signalCallback);
        ~MyUnixSignalHandlerImpl();
        void Initialise(bool daemonise);
        void InstallHandler(int sig);
        void Daemonise();
        
    private:
        static void Handle(int sig, siginfo_t const& siginfo);
    };
    
}

#endif /* defined(__AMQP__MyUnixSignalHandlerImpl__) */
