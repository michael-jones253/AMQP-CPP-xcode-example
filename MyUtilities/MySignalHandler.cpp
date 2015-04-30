//
//  MySignalHandler.cpp
//  AMQP
//
//  Created by Michael Jones on 29/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MySignalHandler.h"
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
    _handlers{} {
        
    }
    
    MySignalHandler::~MySignalHandler() {
        // RAII to ensure that once this destructs signals will not attempt to access this.
        
        /* Reset flags to use sa_handler instead of sa_sigaction. */
        memset (&act, '\0', sizeof(act));
        
        /* Instruct sigaction to reset system handlers back to default */
        act.sa_handler = SIG_DFL;
        
        for (auto & entry : _handlers) {
            sigaction(entry.first, &act, nullptr);
        }
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
        if (daemonise) {
            Daemonise();
        }
        
        _processGroupId = setsid();
        
        _processGroupId = getpid();
        
        
        memset (&act, '\0', sizeof(act));
        
        /* Use the sa_sigaction field because the handles has two additional parameters */
        act.sa_sigaction = &SignalHandler;
        
        /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
        act.sa_flags = SA_SIGINFO;

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
        if (sigaction(sig, &act, nullptr) < 0) {
            perror ("sigaction for TERM");
            return;
        }

        _handlers[sig] = handler;
    }
    
    void MySignalHandler::Daemonise() {
        
        auto ret = fork();
        
        if (ret < 0) {
            // Error
            exit(-1);
        }
        else if (ret > 1) {
            // Parent.
            cout << "Parent exits" << endl;
            exit(0);
        }        
    }
    
    void MySignalHandler::Handle(int sig, int signallingProcessId) {
        auto handler = _handlers.find(sig);
        
        if (handler == _handlers.end()) {
            cerr << " No handler for signal." << endl;
            return;
        }
        
        auto locked = handler->second.lock();
        
        (*locked)(_processId == signallingProcessId, _processId == _processGroupId);
    }
    

}

