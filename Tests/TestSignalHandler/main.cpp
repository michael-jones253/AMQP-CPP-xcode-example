//
//  main.cpp
//  TestSignalHandler
//
//  Created by Michael Jones on 29/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include <iostream>

#include "MySignalCallbacks.h"
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace MyUtilities;
using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    // Singletons can make it very difficult for the coder to determine destruction order of globals, sometimes with
    // unexpected results. See RAII signal ownership below to add some clear determination to signal handling release.
    MySignalHandler::Instance()->Initialise(false);
    
    bool caughtTerminate{};
    mutex signalMutex{};
    condition_variable signalCondition{};
    
    auto termHandler = [&](bool x, bool y) {
        cerr << "SIGTERM - restoring default handling." << endl;
        {
            lock_guard<mutex> guard(signalMutex);
            caughtTerminate = true;
        }
        signalCondition.notify_one();
        
    };
    
    auto hupHandler = [](bool, bool) {
        cerr << "SIGHUP" << endl;
    };
    
    auto ctrlCHandler = [](bool, bool) {
        cerr << "CTRL-C" << endl;
    };

    auto pipeHandler = [](bool, bool) {
        cerr << "PIPE" << endl;
    };
    
    {
        // RAII release of signal handlers from OS.
        MySignalCallbacks signalCallbacks;
        
        signalCallbacks.InstallTerminateHandler(termHandler);
        signalCallbacks.InstallReloadHandler(hupHandler);
        signalCallbacks.InstallCtrlCHandler(ctrlCHandler);
        signalCallbacks.InstallBrokenPipeHandler(pipeHandler);
        
        cout << "Send SIGTERM to restore default handling" << endl;
        
        unique_lock<mutex> lock(signalMutex);
        signalCondition.wait(lock, [&]() { return caughtTerminate; });
    }
    
    for (int x = 0; x < 60; ++x) {
        sleep_for(seconds(2));
        cout << "default handling: Ctrl-C to exit." << endl;
    }
    
    return 0;
}
