//
//  MyTaskProcessor.cpp
//  AMQP
//
//  Created by Michael Jones on 23/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyTaskProcessor.h"
#include <iostream>
#include <assert.h>

using namespace std;

namespace MyAMQP {
    MyTaskProcessor::MyTaskProcessor() :
    _taskQueue{},
    _loopHandle{},
    _shouldRun{}
    {
        
    }
    
    MyTaskProcessor::~MyTaskProcessor() {
        try {
            Stop(false);
        } catch (exception const& ex) {
            cerr << "MyTaskProcessor destruction: " << ex.what() << endl;
        }
    }
    
    void MyTaskProcessor::Start() {
        _shouldRun = true;
        _taskQueue.Reset();
        _loopHandle = async(launch::async, [this]() { return ProcessLoop(); });
    }
    
    void MyTaskProcessor::Stop(bool flush) {
        if (!_loopHandle.valid()) {
            return;
        }
        
        _shouldRun = false;
        _taskQueue.BreakWait();
        
        auto ret = _loopHandle.get();
        
        if (ret < 0) {
            cout << "MyTaskProcessor abnormal exit" << endl;
        }
        
        if (flush) {
            Flush();
        }
    }
    
    void MyTaskProcessor::Push(std::packaged_task<int64_t(void)>&& task) {
        _taskQueue.Push(move(task));
    }
    
    int MyTaskProcessor::ProcessLoop() {
        
        if (!_shouldRun) {
            return -1;
        }
        
        while (_shouldRun) {
            packaged_task<int64_t(void)> task{};
            
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
    
    void MyTaskProcessor::Flush() {
        while (!_taskQueue.Empty()) {
            packaged_task<int64_t(void)> task{};
            _taskQueue.Pop(task);
            
            assert(task.valid());
            
            // Execute the task. This blocks.
            task();
        }
    }
    
}