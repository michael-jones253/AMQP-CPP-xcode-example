//
//  main.cpp
//  TestSignalHandler
//
//  Created by Michael Jones on 29/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include <iostream>

#include "MySignalHandler.h"
#include <thread>
#include <chrono>

using namespace MyUtilities;
using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    MySignalHandler::Instance()->Initialise(false);
    
    auto termHandler = [](bool x, bool y) {
        cerr << "SIGTERM" << endl;
    };
    
    auto hupHandler = [](bool, bool) {
        cerr << "SIGHUP" << endl;
    };
    
    auto term = make_shared<SignalCallback const>(termHandler);
    auto hup = make_shared<SignalCallback const>(hupHandler);
    
    MySignalHandler::Instance()->InstallTermHandler(term);
    MySignalHandler::Instance()->InstallHupHandler(term);
    
    for (int x = 0; x < 100; ++x) {
        sleep_for(seconds(1));
        cout << "working" << endl;
    }
    
    return 0;
}
