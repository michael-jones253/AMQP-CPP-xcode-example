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
#include <amqpcpp.h>

using namespace std;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    auto inst = unique_ptr<AMQPSTL>(new AMQPSTL());
    inst->HelloWorld("Hi");
    
    AMQP::Connection msg{nullptr};
    msg.~Connection();
    
    return 0;
}
