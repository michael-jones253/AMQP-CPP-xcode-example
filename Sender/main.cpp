//
//  main.cpp
//  Sender
//
//  Created by Michael Jones on 17/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include <iostream>
#include <memory>
#include "MyAMQPClient.h"
#include "MyNetworkConnection.h"
#include <amqpcpp.h>

#include <thread>
#include <chrono>

using namespace MyAMQP;
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

int main(int argc, const char * argv[]) {
    // insert code here...
    try {
        auto netConnection = unique_ptr<MyNetworkConnection>(new MyNetworkConnection());
        
        MyAMQPClient myAmqp{move(netConnection)};

        myAmqp.Open("127.0.0.1");
        
        myAmqp.HelloChannel();
        
        while (true) {
            myAmqp.HelloWorld("sawadee krup");
            sleep_for(seconds(1));
        }
    } catch (exception& ex) {
        cerr << ex.what() << endl;
    }
    
    return 0;
}
