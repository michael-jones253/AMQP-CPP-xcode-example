//
//  MyRequestProcessor.cpp
//  AMQP
//
//  Created by Michael Jones on 5/05/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyRequestProcessor.h"
#include <iostream>
#include <assert.h>

using namespace std;

namespace MyAMQP {
    MyRequestProcessor::MyRequestProcessor() :
    _taskQueue{},
    _loopHandle{},
    _shouldRun{}
    {
        
    }
    
    MyRequestProcessor::~MyRequestProcessor() {
        try {
            Stop();
        } catch (exception const& ex) {
            cerr << "MyRequestProcessor destruction: " << ex.what() << endl;
        }
    }
    
    void MyRequestProcessor::Start() {
        _shouldRun = true;
        _taskQueue.Reset();
        _loopHandle = async(launch::async, [this]() { return ProcessLoop(); });
    }
    
    void MyRequestProcessor::Stop() {
        if (!_loopHandle.valid()) {
            return;
        }
        
        _shouldRun = false;
        _taskQueue.BreakWait();
        
        auto ret = _loopHandle.get();
        
        if (ret < 0) {
            cout << "MyRequestProcessor abnormal exit" << endl;
        }
    }
    
    bool MyRequestProcessor::Push(std::packaged_task<ssize_t(void)>&& task) {
        if (!_shouldRun) {
            // Discard further messages once stop is called.
            return false;
        }
        
        _taskQueue.Push(move(task));
        return true;
    }
    
    int MyRequestProcessor::ProcessLoop() {
        
        if (!_shouldRun) {
            return -1;
        }
        
        while (_shouldRun) {
            packaged_task<ssize_t(void)> task{};
            
            auto ok = _taskQueue.Wait(task);
            
            if (!ok) {
                break;
            }
            
            assert(task.valid());
            
            // Does not throw. The get of the future gets any task exceptions.
            task();
        }
        
        return 0;
    }
}
