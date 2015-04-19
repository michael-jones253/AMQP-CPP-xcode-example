//
//  MyAMQPNetworkConnection.cpp
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyAMQPNetworkConnection.h"

#include <stdio.h>
#include <assert.h>
#include <iostream>

using namespace std;
namespace MyAMQP {
    
    
    MyAMQPNetworkConnection::MyAMQPNetworkConnection() :
    _onBytes{},
    _onError{},
    _readLoopHandle{},
    _readShouldRun{},
    _amqpBuffer{}
    {
        
    }
    
    MyAMQPNetworkConnection::~MyAMQPNetworkConnection() {
        try {
            Close();
        }
        catch(exception& ex)
        {
            cerr << "Exception in MyAMQPNetworkConnection destruction: " << ex.what() << endl;
        }
    }
    
    void MyAMQPNetworkConnection::Open(std::string const& ipAddress, std::function<size_t(char const* buf, ssize_t len)> onBytes, std::function<void(std::string const& errString)> onError) {
        
        // Make robust to multiple opens.
        Close();
        
        _onBytes = onBytes;
        _onError = onError;
        
        Connect(ipAddress, 5672);
        
        _readShouldRun = true;
        
        auto readLoop = [this]() ->int {
            auto errCode = int{};
            try {
                ReadLoop();
            } catch (exception& ex) {
                errCode = -1;
                _onError(ex.what());
            }
            
            return errCode;
        };
        
        auto handle = async(launch::async, readLoop);
        _readLoopHandle = move(handle);
    }
    
    void MyAMQPNetworkConnection::Close() {
        if (!_readShouldRun) {
            // Ensure robust to multiple closes.
            return;
        }
        
        _readShouldRun = false;
        Disconnect();

        // Wait for async task to exit. (Equivalent of thread join).
        auto exitCode = _readLoopHandle.get();
        if (exitCode < 0) {
            cerr << "Read loop exited with error code" << endl;
        }
        
    }
    
    void MyAMQPNetworkConnection::ReadLoop() {
        
        auto networkReadFn = [this](char* buf, ssize_t len)->ssize_t {
            auto ret = Read(buf, len);
            
            // Should throw on socket error.
            assert(ret >= 0);
            
            return ret;
        };
        
        ssize_t const amountToAppendFromRead = 1024;
        
        while (_readShouldRun) {
            _amqpBuffer.AppendBack(networkReadFn, amountToAppendFromRead);
            auto parsedBytes = _onBytes(_amqpBuffer.Get(), _amqpBuffer.Count());
            _amqpBuffer.ConsumeFront(parsedBytes);
            
        }        
    }
}