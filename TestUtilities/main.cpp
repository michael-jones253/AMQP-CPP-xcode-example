//
//  main.cpp
//  TestUtilities
//
//  Created by Michael Jones on 25/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyStopwatch.h"
#include <assert.h>
#include <iostream>
#include <thread>

using namespace MyUtilities;
using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    MyStopwatch stopwatch{};
    
    sleep_for(seconds(1));
    
    auto elapsed = stopwatch.GetElapsedMilliseconds();
    assert(elapsed == milliseconds{} && "Stopwatch must not register some elapsed if never started.");
    
    stopwatch.Start();
    sleep_for(milliseconds(10));
    elapsed = stopwatch.GetElapsedMilliseconds();
    assert(elapsed > milliseconds{} && "Stopwatch must have registered some elapsed after starting.");
    
    stopwatch.Start();
    sleep_for(milliseconds(10));
    elapsed = stopwatch.GetElapsedMilliseconds();
    assert(elapsed >= milliseconds{20} && "Time should keep accumulating unless the Restart method is called");
    
    stopwatch.Restart();
    sleep_for(milliseconds(10));
    elapsed = stopwatch.GetElapsedMilliseconds();
    assert(elapsed >= milliseconds{10} && elapsed < milliseconds{20} && "Time should be reset after Restart is called");
    
    stopwatch.Stop();
    sleep_for(milliseconds(20));
    assert(elapsed >= milliseconds{10} && elapsed < milliseconds{20} && "Time must not advance after Stop is called");
    
    stopwatch.Start();
    sleep_for(milliseconds(10));
    assert(elapsed >= milliseconds{10} && elapsed < milliseconds{20} && "Time must be reset after Stop is called");
    
    return 0;
}
