//
//  MySignalHandler.cpp
//  AMQP
//
//  Created by Michael Jones on 29/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MySignalHandler.h"

#if defined(__APPLE__)
#include "MyUnixSignalHandlerImpl.h"
#endif

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <functional>
#include <signal.h>

using namespace std;
using namespace std::placeholders;

namespace {
    static struct sigaction act;

}

namespace MyUtilities {
    
    void SignalHandler(int sig, siginfo_t *siginfo, void *context) {
        cout << "Caught signal:" << sig << endl;
        MyUtilities::MySignalHandler::Instance()->Handle(sig, siginfo->si_pid);
    }

    // Singletons are usually BAD, but we don't have much choice with signal handling.
    std::unique_ptr<MySignalHandler> MySignalHandler::Singleton{};
    
    MySignalHandler::MySignalHandler() :
    _processId{},
    _processGroupId{},
    _handlers{},
    _impl{} {
        auto signalHandler = bind(&MySignalHandler::Handle, this, placeholders::_1, placeholders::_2);
        
#if defined(__APPLE__)
        _impl = unique_ptr<MyUnixSignalHandlerImpl>( new MyUnixSignalHandlerImpl(signalHandler) );
#endif
        
    }
    
    MySignalHandler::~MySignalHandler() {
#if defined(FIXME)
        // RAII to ensure that once this destructs signals will not attempt to access this.
        
        /* Reset flags to use sa_handler instead of sa_sigaction. */
        memset (&act, '\0', sizeof(act));
        
        /* Instruct sigaction to reset system handlers back to default */
        act.sa_handler = SIG_DFL;
        
        for (auto & entry : _handlers) {
            sigaction(entry.first, &act, nullptr);
        }
#endif
    }
    
    MySignalHandler* MySignalHandler::Instance() {
        if (Singleton == nullptr) {
            Singleton = unique_ptr<MySignalHandler>( new MySignalHandler);
        }
        
        return Singleton.get();
    }
    
    bool MySignalHandler::IsInitialised() {
        return Singleton != nullptr;
    }

    void MySignalHandler::Reset() {
        Singleton.reset();
    }
    
    void MySignalHandler::Initialise(bool daemonise) {
        /*
        if (daemonise) {
            
            Daemonise();
        }
         */
        
        _impl->Initialise(daemonise);
#if defined(FIXME)
        
        _processGroupId = setsid();
        
        _processGroupId = getpid();
        
        
        memset (&act, '\0', sizeof(act));
        
        /* Use the sa_sigaction field because the handles has two additional parameters */
        act.sa_sigaction = &SignalHandler;
        
        /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
        act.sa_flags = SA_SIGINFO;
#endif

    }
    
    void MySignalHandler::InstallReloadHandler(std::shared_ptr<SignalCallback const> handler) {
        InstallHandler(SIGHUP, handler);
    }
    
    void MySignalHandler::InstallBrokenPipeHandler(std::shared_ptr<SignalCallback const> handler) {
        InstallHandler(SIGPIPE, handler);
    }
    
    void MySignalHandler::InstallTerminateHandler(std::shared_ptr<SignalCallback const> handler) {
        InstallHandler(SIGTERM, handler);
    }

    void MySignalHandler::InstallCtrlCHandler(std::shared_ptr<SignalCallback const> handler) {
        InstallHandler(SIGINT, handler);
    }
    
    void MySignalHandler::InstallHandler(int sig, std::shared_ptr<SignalCallback const> handler) {
        /*
        if (sigaction(sig, &act, nullptr) < 0) {
            perror ("sigaction for TERM");
            return;
        }
         */
        _impl->InstallHandler(sig);

        _handlers[sig] = handler;
    }
    
    void MySignalHandler::Daemonise() {
        // FIX ME not needed.
        _impl->Daemonise();
    }
    
    void MySignalHandler::Handle(int sig, int signallingProcessId) {
        auto handler = _handlers.find(sig);
        
        if (handler == _handlers.end()) {
            cerr << " No handler for signal." << endl;
            return;
        }
        
        auto locked = handler->second.lock();
        if (locked) {
            (*locked)(_processId == signallingProcessId, _processId == _processGroupId);
        }
    }
    

}

