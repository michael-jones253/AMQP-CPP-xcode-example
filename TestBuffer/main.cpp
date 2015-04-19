//
//  main.cpp
//  TestBuffer
//
//  Created by Michael Jones on 19/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include <iostream>

#include "MyAMQPBuffer.h"
#include "string.h"
#include <string>

using namespace MyAMQP;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    
    MyAMQPBuffer buf;
    

    // Deliberately no try, so that debugger will catch.
    auto available = buf.Available();
    
    memset(const_cast<char*>(buf.Get()), 0xFF, available);
    
    auto readFn = [&](char* buffer, ssize_t maxLen)->ssize_t {
        memset(const_cast<char*>(buf.Get()), 0xFF, maxLen);
        return maxLen;
    };
    
    buf.AppendBack(readFn, 99);
    
    return 0;
}
