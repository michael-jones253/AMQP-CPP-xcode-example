//
//  MyAMQPNetworkConnection.cpp
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyAMQPNetworkConnection.h"

#include <stdio.h>

using namespace std;

MyAMQPNetworkConnection::MyAMQPNetworkConnection() :
    _onBytes{},
    _onError{},
    _readLoopHandle{}
{
    
}

void MyAMQPNetworkConnection::Open(std::string const& ipAddress, std::function<int(char const* buf, int len)> onBytes, std::function<void(std::string const& errString)> onError) {
    _onBytes = onBytes;
    _onError = onError;
    
    Connect(ipAddress, 5672);
    
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
    Disconnect();
}

void MyAMQPNetworkConnection::ReadLoop() {
    while (true) {
        char buf[1024];
        
        // FIX ME let it throw.
        auto ret = Read(buf, sizeof(buf));
        if (ret < 0) {
            break;
        }
        
        auto parsedBytes = _onBytes(buf, ret);
        if (parsedBytes < ret) {
            // FIX ME - need to buffer.
        }
    }

}
