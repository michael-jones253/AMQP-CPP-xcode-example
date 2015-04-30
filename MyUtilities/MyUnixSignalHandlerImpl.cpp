//
//  MyUnixSignalHandlerImpl.cpp
//  AMQP
//
//  Created by Michael Jones on 30/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyUnixSignalHandlerImpl.h"

#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;

namespace MyUtilities {
    
    void UnixSignalHandler(int sig, siginfo_t *siginfo, void *context) {
        cout << "Caught signal:" << sig << endl;
        MyUnixSignalHandlerImpl::Handle(sig, *siginfo);
    }
    
    struct sigaction MyUnixSignalHandlerImpl::SigAct{};
    SignalFunction MyUnixSignalHandlerImpl::SignalCallback{};
    int MyUnixSignalHandlerImpl::ProcessGroupId{};
    int MyUnixSignalHandlerImpl::ProcessId{};
            
    MyUnixSignalHandlerImpl::MyUnixSignalHandlerImpl(SignalFunction const& signalCallback) :
    _handledSignals{}
    {
        SignalCallback = signalCallback;
    }
    
    MyUnixSignalHandlerImpl::~MyUnixSignalHandlerImpl() {
        // RAII to ensure that once this destructs signals will not attempt to access this.
        
        /* Reset flags to use sa_handler instead of sa_sigaction. */
        memset (&SigAct, '\0', sizeof(SigAct));
        
        /* Instruct sigaction to reset system handlers back to default */
        SigAct.sa_handler = SIG_DFL;
        
        for (auto & entry : _handledSignals) {
            sigaction(entry, &SigAct, nullptr);
        }
        
    }
    
    void MyUnixSignalHandlerImpl::Initialise(bool daemonise) {
        if (daemonise) {
            Daemonise();
        }
        
        ProcessGroupId = setsid();
        
        ProcessId = getpid();
        
        memset (&SigAct, '\0', sizeof(SigAct));
        
        /* Use the sa_sigaction field because the handles has two additional parameters */
        SigAct.sa_sigaction = &UnixSignalHandler;
        
        /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
        SigAct.sa_flags = SA_SIGINFO;
        
    }
    
    void MyUnixSignalHandlerImpl::InstallHandler(int sig) {
        if (sigaction(sig, &SigAct, nullptr) < 0) {
            auto err = string{"sigaction for: "} + to_string(sig);
            perror (err.c_str());
            return;
        }
        
        _handledSignals.insert(sig);
    }
    
    void MyUnixSignalHandlerImpl::Daemonise() {
        
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
    
    void MyUnixSignalHandlerImpl::Handle(int sig,  siginfo_t const& siginfo) {
        SignalCallback(sig, ProcessId == siginfo.si_pid, ProcessGroupId == siginfo.si_pid);
    }
    
}
