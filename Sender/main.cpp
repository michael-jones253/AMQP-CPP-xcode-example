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
#include "../MyNetworkConnection/MyUnixNetworkConnection.h"

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
            { "exchange",     required_argument,            nullptr,'e'},
            { "key",     required_argument,            nullptr,     'k'},
            { "queue",     required_argument,            nullptr,   'q'},
            { "count",   required_argument,      nullptr,           'c' },
            { "receivers", required_argument,      nullptr,           'r' },
            { "sleep",   required_argument,      nullptr,           's' },
            { NULL,    0,                 nullptr,           0 }
        };
        
        MyLoginCredentials loginInfo{"127.0.0.1", "guest", "guest"};
        
        // Default exchange type.
        AMQP::ExchangeType exchangeType{AMQP::ExchangeType::direct};
        
        MyAMQPRoutingInfo routingInfo{"my_exchange", "key", "my_queue"};
        
        int messageCount{100};
        int sleepSeconds{};
        
        // We send multiple end messages, because each end message is only processed by one consumer
        // sending of multiple end messages means that multiple consumers can receive an end message.
        int receiverCount = 4;

        auto const usageStr = string("Usage: [--fanout | --topic {default direct}] [--count <integer>] [--sleep <seconds>]");
        
        do {
            auto ch = getopt_long(argc, (char* const *)(argv), "fte:k:q:c:s:", longopts, nullptr);
            
            switch (ch) {
                case 'f':
                    exchangeType = AMQP::ExchangeType::fanout;
                    break;
                    
                case 't':
                    // FIX ME - exchange type topic will need a routing key argument.
                    throw runtime_error("Exchange type topic not implemented yet");
                    
                case 'e':
                    routingInfo.ExchangeName = optarg;
                    break;
                    
                case 'k':
                    routingInfo.Key = optarg;
                    break;
                    
                case 'q':
                    routingInfo.QueueName = optarg;
                    break;
                    
                case 'c':
                    messageCount = stoi(optarg);
                    break;

                case 'r':
                    receiverCount = stoi(optarg);
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
        
        
        auto netConnection = unique_ptr<MyUnixNetworkConnection>(new MyUnixNetworkConnection());
        
        MyAMQPClient myAmqp{move(netConnection)};
        
        myAmqp.Open(loginInfo);
        
        myAmqp.CreateHelloQueue(exchangeType, routingInfo);
        
        auto startTime = system_clock::now();
        
        for (int count = 0; count < messageCount; count++) {
            myAmqp.SendHelloWorld(routingInfo.ExchangeName, routingInfo.Key, "sawasdee krup");
            
            sleep_for(seconds(sleepSeconds));
        }
        
        for (int x = 0; x < receiverCount; ++x) {
            myAmqp.SendHelloWorld(routingInfo.ExchangeName, routingInfo.Key, "end");
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
