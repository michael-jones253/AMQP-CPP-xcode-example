//
//  main.cpp
//  TestBufferedConnection
//
//  Created by Michael Jones on 29/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyAMQPBufferedConnection.h"
#include "MyUnixNetworkConnection.h"
#include "MyNetworkException.h"
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <assert.h>

using namespace MyAMQP;
using namespace MyUtilities;
using namespace std;
using namespace std::chrono;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    auto unixConnection = unique_ptr<MyUnixNetworkConnection>(new MyUnixNetworkConnection{});
    
    MyAMQPBufferedConnection bufferedConnection{move(unixConnection)};
    
    try {
        
        bool notifyError{};
        condition_variable notifyCondition{};
        mutex notifyMutex{};
        
        auto parseBytesCallback = [](char const* buf, ssize_t len) -> size_t {
            cout << buf << endl;
            return len;
        };
        
        auto networkErrorCallback = [&](std::string const& errString) {
            cerr << "Buffered connection error: " << errString << endl;
            
            notifyError = true;
            notifyCondition.notify_one();
        };
        
        bufferedConnection.SetCallbacks(parseBytesCallback, networkErrorCallback);
        try {
            bufferedConnection.Open("127.0.0.1");
        } catch (MyNetworkException const& ex) {
            cerr << "This test requires a running local RabbitMQ server" << ex.what() << endl;
            assert(false && "Must have started the server first");
        }
        
        
        string junk{"xxx"};
        
        bufferedConnection.SendToServer(junk.data(), junk.length());
        
        unique_lock<mutex> lock(notifyMutex);
        
        auto timeoutForErrorNotification = seconds(20);
        auto ok = notifyCondition.wait_for(lock, timeoutForErrorNotification, [&]() { return notifyError; } );
        
        assert(ok && "Not expecting a timeout while waiting for error notification.");
        
        bool threwOnWrite{};
        try {
            bufferedConnection.Close();
            bufferedConnection.SendToServer(junk.data(), junk.length());
        } catch (MyNetworkException const& ex) {
            cerr << ex.what() << endl;
            threwOnWrite = true;
        }
        
        assert(threwOnWrite && "Expect exception on write on closed connection.");
        
    } catch (exception const& ex) {
        cerr << "Unexpected exception in buffered connection test: " << ex.what() << endl;
        assert(false && "Unexpected exception in buffered connection test");
    }
    
    return 0;
}
