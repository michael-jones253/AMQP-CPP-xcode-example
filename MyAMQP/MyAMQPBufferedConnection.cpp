//
//  MyAMQPBufferedConnection.cpp
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyAMQPBufferedConnection.h"

#include <stdio.h>
#include <assert.h>
#include <iostream>

using namespace std;
namespace MyAMQP {
    
    
    MyAMQPBufferedConnection::MyAMQPBufferedConnection(
                                                       std::unique_ptr<MyNetworkConnection> networkConnection,
                                                       ParseBytesCallback const& parseBytesCallback,
                                                       NetworkErrorCallback const & onErrorCallback) :
    _networkConnection(move(networkConnection)),
    _parseReceivedBytes{parseBytesCallback},
    _onError{onErrorCallback},
    _readLoopHandle{},
    _readShouldRun{},
    _amqpBuffer{}
    {
        
    }
    
    MyAMQPBufferedConnection::~MyAMQPBufferedConnection() {
        try {
            Close();
        }
        catch(exception& ex)
        {
            cerr << "Exception in MyAMQPBufferedConnection destruction: " << ex.what() << endl;
        }
    }
    
    void MyAMQPBufferedConnection::Open(std::string const& ipAddress) {
    
        // Make robust to multiple opens.
        Close();
        
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
    
    void MyAMQPBufferedConnection::Close() {
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
    
    void MyAMQPBufferedConnection::Connect(std::string const& ipAddress, int port) {
        _networkConnection->Connect(ipAddress, port);
    }
    
    void MyAMQPBufferedConnection::Disconnect() {
        _networkConnection->Disconnect();
    }
    
    ssize_t MyAMQPBufferedConnection::Read(char *buf, size_t len) {
        return _networkConnection->Read(buf, len);
    }
    
    void MyAMQPBufferedConnection::WriteAll(char const* buf, size_t len) {
        _networkConnection->WriteAll(buf, len);
    }
    
    void MyAMQPBufferedConnection::ReadLoop() {
        
        auto networkReadFn = [this](char* buf, ssize_t len)->ssize_t {
            auto ret = Read(buf, len);
            
            // Should throw on socket error.
            assert(ret >= 0);
            
            return ret;
        };
        
        ssize_t const amountToAppendFromRead = 1024;
        
        while (_readShouldRun) {
            // Circular processing - add to the end from network reads, consume from the front via the parse callback.
            _amqpBuffer.AppendBack(networkReadFn, amountToAppendFromRead);
            auto parsedBytes = _parseReceivedBytes(_amqpBuffer.Get(), _amqpBuffer.Count());
            _amqpBuffer.ConsumeFront(parsedBytes);
            
        }        
    }
}