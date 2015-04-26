//
//  MyStopwatch.cpp
//  AMQP
//
//  Created by Michael Jones on 25/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyStopwatch.h"

using namespace std;
using namespace std::chrono;

namespace MyUtilities {
    MyStopwatch::MyStopwatch() :
    _isRunning{},
    _startTime{},
    _endTime{}
    {
    }
    
    void MyStopwatch::Start() {
        if (_isRunning) {
            return;
        }
        
        _startTime = high_resolution_clock::now();
        _isRunning = true;
    }
    
    void MyStopwatch::Restart() {
        Stop();
        Start();
    }
    
    void MyStopwatch::Stop() {
        if (!_isRunning) {
            return;
        }
        
        _endTime = high_resolution_clock::now();
        _isRunning = false;
    }
    
    milliseconds MyStopwatch::GetElapsedMilliseconds() const {
        auto elapsed = _isRunning ? high_resolution_clock::now() - _startTime : _endTime - _startTime;
        
        return duration_cast<milliseconds>(elapsed);
    }
}