//
//  main.cpp
//  Sender
//
//  Created by Michael Jones on 17/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include <iostream>
#include <memory>
#include "AMQPSTL.h"
#include "MyAMQP.h"
#include "MyNetworkConnection.h"
#include <amqpcpp.h>

#include <thread>
#include <chrono>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    auto inst = unique_ptr<AMQPSTL>(new AMQPSTL());
    inst->HelloWorld("Hi");
    
    auto netConnection = unique_ptr<MyNetworkConnection>(new MyNetworkConnection());
    
    MyAMQP myAmqp{move(netConnection)};

    myAmqp.Open("127.0.0.1");
    
    myAmqp.HelloChannel();
    
    while (true) {
        myAmqp.HelloWorld("sawadee krup");
        sleep_for(seconds(1));
    }
    
    return 0;
}
