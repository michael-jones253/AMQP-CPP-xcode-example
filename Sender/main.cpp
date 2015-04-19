//
//  main.cpp
//  Sender
//
//  Created by Michael Jones on 17/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <memory>
#include "MyAMQPClient.h"
#include "MyNetworkConnection.h"
#include <amqpcpp.h>

#include <thread>
#include <chrono>
#include <getopt.h>

using namespace MyAMQP;
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

int main(int argc, const char * argv[]) {
    // insert code here...
    try {
        /* options descriptor */
        static struct option longopts[] = {
            { "fanout",  no_argument,            nullptr,           'f'},
            { "topic",   no_argument,            nullptr,           't'},
            { "count",   required_argument,      nullptr,           'c' },
            { "sleep",   required_argument,      nullptr,           's' },
            { NULL,    0,                 nullptr,           0 }
        };
        
        // Default exchange type.
        AMQP::ExchangeType exchangeType{AMQP::ExchangeType::direct};
        
        int messageCount{100};
        int sleepSeconds{};
        
        auto const usageStr = string("Usage: [--fanout | --topic {default direct}] [--count <integer>] [--sleep <seconds>]");
        
        do {
            auto ch = getopt_long(argc, (char* const *)(argv), "ftc:s:", longopts, nullptr);
            
            switch (ch) {
                case 'f':
                    exchangeType = AMQP::ExchangeType::fanout;
                    break;
                    
                case 't':
                    // FIX ME - exchange type topic will need a routing key argument.
                    throw runtime_error("Exchange type topic not implemented yet");
                    
                case 'c':
                    messageCount = stoi(optarg);
                    break;
                    
                case 's':
                    sleepSeconds = stoi(optarg);
                    
                case -1:
                    break;
                    
                default:
                    throw runtime_error(usageStr);
                    break;
            }
            
            if (ch == -1) {
                break;
            }
            
        } while (true);
        
        
        auto netConnection = unique_ptr<MyNetworkConnection>(new MyNetworkConnection());
        
        MyAMQPClient myAmqp{move(netConnection)};
        
        myAmqp.Open("127.0.0.1");
        
        myAmqp.CreateHelloQueue(exchangeType);
        
        auto startTime = system_clock::now();
        
        for (int count = 0; count < messageCount; count++) {
            myAmqp.SendHelloWorld("sawasdee krup");
            
            sleep_for(seconds(sleepSeconds));
        }
        
        // Benchmark if sending out one after another.
        if (sleepSeconds == 0) {
            auto elapsed = system_clock::now() - startTime;
            auto elapsedMs = duration_cast<milliseconds>(elapsed);
            stringstream messageStr;
            messageStr << messageCount << " messages sent in: " << elapsedMs.count() << " ms";
            
            cout << messageStr.str() << endl;
        }
        
        
        myAmqp.Close();
        
    } catch (exception& ex) {
        cerr << ex.what() << endl;
    }
    
    return 0;
}
