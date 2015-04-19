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
using namespace std;

int main(int argc, const char * argv[]) {
    // insert code here...
    cout << "Hello, World!\n";
    
    
    MyAMQPBuffer buf;
    
    int const ParsableFrameSize = 99;
    
    auto testParseFn = [](char const* buffer, ssize_t len) -> ssize_t {
        if (len >= ParsableFrameSize) {
            return ParsableFrameSize;
        }
        
        return 0;
    };
    
    auto testParsePartialFn = [](char const* buffer, ssize_t len) -> ssize_t {
        if (len >= ParsableFrameSize) {
            return ParsableFrameSize;
        }
        
        return 11;
    };
    
    auto readFn = [&](char* buffer, ssize_t maxLen)->ssize_t {
        memset(const_cast<char*>(buf.Get()), 0xFF, maxLen);
        return maxLen;
    };
    
    for (int x = 0; x < 20; x++) {
        buf.AppendBack(readFn, 11);
        
        auto ret = testParseFn(buf.Get(), buf.Count());
        cout << "parsed: " << ret << endl;
        
        buf.ConsumeFront(ret);
        
        if (buf.Count() == 0) {
            cout << "Success buffer emptied" << endl;
            break;
        }
    }
    
    for (int x = 0; x < 20; x++) {
        buf.AppendBack(readFn, 8);
        
        auto ret = testParseFn(buf.Get(), buf.Count());
        cout << "parsed: " << ret << endl;
        
        buf.ConsumeFront(ret);
        
        if (buf.Count() < 8) {
            cout << "Consume success, Remainder: " << buf.Count() << endl;
            break;
        }
    }
    
    for (int x = 0; x < 20; x++) {
        buf.AppendBack(readFn, 22);
        
        auto ret = testParsePartialFn(buf.Get(), buf.Count());
        cout << "parsed: " << ret;
        cout << ", count:" << buf.Count() << endl;
        
        buf.ConsumeFront(ret);

        if (buf.Count() < 11) {
            cout << "Consume partial parse success, Remainder: " << buf.Count() << endl;
            break;
        }        
    }
    
    return 0;
}
