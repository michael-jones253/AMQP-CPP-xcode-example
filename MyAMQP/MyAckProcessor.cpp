//
//  MyAckProcessor.cpp
//  AMQP
//
//  Created by Michael Jones on 23/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyAckProcessor.h"
#include "MyNetworkException.h"

#include <iostream>
#include <chrono>

#include <assert.h>

using namespace MyUtilities;
using namespace std;
using namespace std::chrono;

namespace MyAMQP {
    MyAckProcessor::MyAckProcessor() :
    _taskQueue{},
    _loopHandle{},
    _ackHandler{},
    _shouldRun{},
    _flushRequested{}
    {
        
    }
    
    MyAckProcessor::~MyAckProcessor() {
        try {
            Stop(false);
        } catch (exception const& ex) {
            cerr << "MyAckProcessor destruction: " << ex.what() << endl;
        }
    }
    
    void MyAckProcessor::Start(function<void(int64_t)> const& ackHandler) {
        _ackHandler = ackHandler;
        Resume();
    }
    
    void MyAckProcessor::Stop(bool flush) {
        if (!_loopHandle.valid()) {
            return;
        }
        
        _shouldRun = false;
        _flushRequested = flush;
        _taskQueue.BreakWait();
        
        auto ret = _loopHandle.get();
        
        if (ret < 0) {
            cout << "MyAckProcessor abnormal exit" << endl;
        }
    }
    
    void MyAckProcessor::Resume() {
        _shouldRun = true;
        _flushRequested = false;
        _taskQueue.Reset();
        _loopHandle = async(launch::async, [this]() { return ProcessLoop(); });
    }
    
    void MyAckProcessor::Push(future<int64_t>&& task) {
        assert(_shouldRun);
        
        if (!_shouldRun) {
            return;
        }
        
        _taskQueue.Push(move(task));
    }
    
    int MyAckProcessor::ProcessLoop() {
        
        if (!_shouldRun) {
            return -1;
        }
        
        while (_shouldRun) {
            future<int64_t> tagResult{};
            
            auto ok = _taskQueue.Wait(tagResult);
            
            if (!ok) {
                break;
            }
            
            assert(tagResult.valid());
            
            // The get might throw, if the user handler threw. In this case we do not ack.
            try {
                auto tag = tagResult.get();
                _ackHandler(tag);
            } catch (exception const& ex) {
                cerr << "MyAckProcessor: " << ex.what() << endl;
            }
        }
        
        if (_flushRequested) {
            Flush();
        }
        
        return 0;
    }
    
    void MyAckProcessor::Flush() {
        int flushed{};
        while (!_taskQueue.Empty()) {
            try {
                future<int64_t> tagResult{};
                _taskQueue.Pop(tagResult);
                
                assert(tagResult.valid());
                auto status = tagResult.wait_for(seconds(5));
                if (status == future_status::timeout) {
                    // Review: this shouldn't be needed.
                    assert(false);
                    cerr << "Not expecting ack flush to timeout" << endl;
                    break;
                }
                
                auto tag = tagResult.get();
                
                _ackHandler(tag);
                ++flushed;
            }
            catch (MyNetworkException const& netEx) {
                // Exception due to network write and not client message handler.
                cerr << "MyAckProcessor Flush: " << netEx.what() << endl;
                throw;
            }
            catch (exception const& ex) {
                // Allow client message handler to throw in the packaged task..
                cerr << "MyAckProcessor Flush: " << ex.what() << endl;
            }
        }
        
        cout << "Flushed: " << flushed << " acks" << endl;
    }
    
}