//
//  main.cpp
//  TestBuffer
//
//  Created by Michael Jones on 19/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include <iostream>

#include "AMQPSTL.h"
#include "MyAMQPCircularBuffer.h"
#include "string.h"
#include <assert.h>
#include <string>

using namespace MyAMQP;
using namespace std;

int main(int argc, const char * argv[]) {
    // insert code here...
    cout << "Hello, World!\n";
    
    try {
        
        // Just a test of library symbol export.
        auto inst = unique_ptr<AMQPSTL>(new AMQPSTL());
        inst->HelloWorld("Hi");
        
        
        MyAMQPCircularBuffer myAmqBuffer;
        
        int const ParsableFrameSize = 99;
        
        auto testParseFn = [](char const* buffer, ssize_t len) -> ssize_t {
            if (len >= ParsableFrameSize) {
                return ParsableFrameSize;
            }
            
            // Parse a complete frame or nothing.
            return 0;
        };
        
        auto testParsePartialFn = [](char const* buffer, ssize_t len) -> ssize_t {
            if (len >= ParsableFrameSize) {
                return ParsableFrameSize;
            }
            
            // Parse a complete frame or partial parse.
            return 11;
        };
        
        auto readFn = [=](char* buffer, ssize_t maxLen)->ssize_t {
            memset(buffer, 0xFF, maxLen);
            return maxLen;
        };
        
        auto completeParseOk = bool{};
        
        for (int x = 0; x < 20; x++) {
            myAmqBuffer.AppendBack(readFn, 11);
            
            auto ret = testParseFn(myAmqBuffer.Get(), myAmqBuffer.Count());
            cout << "parsed: " << ret << endl;
            
            myAmqBuffer.ConsumeFront(ret);
            
            if (myAmqBuffer.Count() == 0) {
                completeParseOk = true;
                cout << "Success buffer emptied" << endl;
                break;
            }
        }
        
        assert(completeParseOk);
        
        auto readWithExtraData = bool{};
        
        for (int x = 0; x < 20; x++) {
            myAmqBuffer.AppendBack(readFn, 8);
            
            auto ret = testParseFn(myAmqBuffer.Get(), myAmqBuffer.Count());
            cout << "parsed: " << ret << endl;
            
            myAmqBuffer.ConsumeFront(ret);
            
            if (myAmqBuffer.Count() < 8) {
                readWithExtraData = true;
                cout << "Consume success, Remainder: " << myAmqBuffer.Count() << endl;
                break;
            }
        }
        
        assert(readWithExtraData);
        
        auto partialParseOK = bool{};
        
        for (int x = 0; x < 20; x++) {
            myAmqBuffer.AppendBack(readFn, 22);
            
            auto ret = testParsePartialFn(myAmqBuffer.Get(), myAmqBuffer.Count());
            cout << "parsed: " << ret;
            cout << ", count:" << myAmqBuffer.Count() << endl;
            
            myAmqBuffer.ConsumeFront(ret);
            
            if (myAmqBuffer.Count() < 11) {
                partialParseOK = true;
                cout << "Consume partial parse success, Remainder: " << myAmqBuffer.Count() << endl;
                break;
            }
        }
        
        assert(partialParseOK);
        
        auto amountBuffered = myAmqBuffer.Buffered();
        
        cout << "Amount buffered: " << myAmqBuffer.Buffered() << endl;
        
        auto compareBuf = unique_ptr<char[]>(new char[amountBuffered]);
        memset(compareBuf.get(), 0xFF, amountBuffered);
        
        // Force a reset of the start of data back to start of underlying storage.
        myAmqBuffer.ConsumeFront(myAmqBuffer.Count());
        
        auto cmp = memcmp(myAmqBuffer.Get(), compareBuf.get(), amountBuffered);
        
        assert(cmp == 0);
        
    }
    catch (exception const& ex) {
        cerr << "Test error: " << ex.what() << endl;
    }
    
    
    return 0;
}
