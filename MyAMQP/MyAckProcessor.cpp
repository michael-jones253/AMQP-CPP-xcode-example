//
//  MyAckProcessor.cpp
//  AMQP
//
//  Created by Michael Jones on 23/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyAckProcessor.h"

#include <iostream>
#include <chrono>

#include <assert.h>

using namespace std;
using namespace std::chrono;

namespace MyAMQP {
    MyAckProcessor::MyAckProcessor() :
    _taskQueue{},
    _loopHandle{},
    _ackHandler{},
    _shouldRun{}
    {
        
    }
    
    MyAckProcessor::~MyAckProcessor() {
        try {
            Stop();
        } catch (exception const& ex) {
            cerr << "MyAckProcessor destruction: " << ex.what() << endl;
        }
    }
    
    void MyAckProcessor::Start(function<void(int64_t)> const& ackHandler) {
        _shouldRun = true;
        _taskQueue.Reset();
        _ackHandler = ackHandler;
        _loopHandle = async(launch::async, [this]() { return ProcessLoop(); });
    }
    
    void MyAckProcessor::Stop() {
        if (!_loopHandle.valid()) {
            return;
        }
        
        _shouldRun = false;
        _taskQueue.BreakWait();
        
        auto ret = _loopHandle.get();
        
        if (ret < 0) {
            cout << "MyAckProcessor abnormal exit" << endl;
        }
    }
    
    void MyAckProcessor::Push(future<int64_t>&& task) {
        _taskQueue.Push(move(task));
    }
    
    int MyAckProcessor::ProcessLoop() {
        
        if (!_shouldRun) {
            return -1;
        }
        
        while (_shouldRun) {
            future<int64_t> task{};
            
            auto ok = _taskQueue.Wait(task);
            
            if (!ok) {
                break;
            }
            
            assert(task.valid());
            
            // The get might throw, if the user handler threw. In this case we do not ack.
            try {
                while (_shouldRun) {
                    // The get() may wait forever if the task never executed, so we protect against that
                    // in the shutdown case.
                    auto ret = task.wait_for(seconds(1));
                    
                    if (ret == future_status::ready) {
                        auto tag = task.get();
                        _ackHandler(tag);
                        
                        // Move on to next future.
                        break;
                    }
                }
            } catch (exception const& ex) {
                cerr << "MyAckProcessor: " << ex.what() << endl;
            }
        }
        
        return 0;
    }
    
}