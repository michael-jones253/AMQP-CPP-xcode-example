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
#include <functional>

using namespace std;
using namespace std::placeholders;

namespace MyUtilities {

    // Singletons are usually BAD, but we don't have much choice with signal handling.
    std::unique_ptr<MySignalHandler> MySignalHandler::Singleton{};
    
    MySignalHandler::MySignalHandler() :
    _handlers{},
    _impl{} {
        auto signalHandler = bind(&MySignalHandler::Handle, this, _1, _2, _3);
        
#if defined(__APPLE__)
        _impl = make_unique<MyUnixSignalHandlerImpl>(signalHandler);
#else
        _impl = make_unique<MyWindowsSignalHandlerImpl>(signalHandler);
#endif
        
    }
    
    MySignalHandler::~MySignalHandler() {
    }
    
    MySignalHandler* MySignalHandler::Instance() {
        if (Singleton == nullptr) {
            // make_unique generates a private constructor error here.
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
        
        _impl->Initialise(daemonise);
    }
    
    void MySignalHandler::InstallReloadHandler(std::shared_ptr<SignalCallback const> handler) {
        InstallHandler(MY_SIGHUP, handler);
    }
    
    void MySignalHandler::InstallBrokenPipeHandler(std::shared_ptr<SignalCallback const> handler) {
        InstallHandler(MY_SIGPIPE, handler);
    }
    
    void MySignalHandler::InstallTerminateHandler(std::shared_ptr<SignalCallback const> handler) {
        InstallHandler(MY_SIGTERM, handler);
    }

    void MySignalHandler::InstallCtrlCHandler(std::shared_ptr<SignalCallback const> handler) {
        InstallHandler(MY_SIGINT, handler);
    }
    
    void MySignalHandler::InstallHandler(int sig, std::shared_ptr<SignalCallback const> handler) {
        _impl->InstallHandler(sig);

        _handlers[sig] = handler;
    }
    
    void MySignalHandler::Handle(int sig, bool fromSelf, bool isDaemon) {
        auto handler = _handlers.find(sig);
        
        if (handler == _handlers.end()) {
            cerr << " No handler for signal." << endl;
            return;
        }
        
        auto locked = handler->second.lock();
        if (locked) {
            (*locked)(fromSelf, isDaemon);
        }
    }
    
}

