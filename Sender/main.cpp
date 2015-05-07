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
#include "MyStopwatch.h"
#include "MyUnixNetworkConnection.h"

#include <thread>
#include <chrono>
#include <getopt.h>

using namespace MyAMQP;
using namespace MyUtilities;
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
            { "receivers", required_argument,      nullptr,         'r' },
            { "sleep",   required_argument,      nullptr,           's' },
            { "bye",     no_argument,            nullptr,     'b'},
            { "noflush", no_argument,            nullptr,           'N'},
            { NULL,    0,                 nullptr,           0 }
        };
        
        MyLoginCredentials loginInfo{"127.0.0.1", "guest", "guest"};
        
        // Default exchange type.
        AMQP::ExchangeType exchangeType{AMQP::ExchangeType::direct};
        
        MyAMQPRoutingInfo routingInfo{"my_exchange", "key", "my_queue"};
        
        int messageCount{100};
        int sleepSeconds{};
        bool sendGoodbyeMessage{};
        bool doFlush{true};
        
        // We send multiple end messages, because each end message is only processed by one consumer
        // sending of multiple end messages means that multiple consumers can receive an end message.
        int receiverCount = 4;

        auto const usageStr = string("Usage: [--fanout | --topic {default direct}] [--count <integer>] [--sleep <seconds>]");
        
        do {
            auto ch = getopt_long(argc, (char* const *)(argv), "fte:k:q:c:s:b", longopts, nullptr);
            
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
                    
                case 'b':
                    sendGoodbyeMessage = true;
                    break;
                    
                case 'N':
                    doFlush = false;
                    break;
                    
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
        
        
        auto netConnection = make_unique<MyUnixNetworkConnection>();
        
        MyAMQPClient myAmqp{move(netConnection)};
        
        myAmqp.Open(loginInfo);
        
        myAmqp.CreateHelloQueue(exchangeType, routingInfo);
        
        MyStopwatch benchmarkStopwatch{};
        benchmarkStopwatch.Start();
        
        for (int count = 0; count < messageCount; count++) {
            myAmqp.SendHelloWorld(routingInfo.ExchangeName, routingInfo.Key, "sawasdee krup");
            
            sleep_for(seconds(sleepSeconds));
        }
        
        for (int x = 0; x < receiverCount; ++x) {
            myAmqp.SendHelloWorld(routingInfo.ExchangeName, routingInfo.Key, "end");
        }
        
        if (sendGoodbyeMessage) {
            myAmqp.SendHelloWorld(routingInfo.ExchangeName, routingInfo.Key, "goodbye");
        }

        // Close before capturing elapsed time, because the close involves flushing of messages out
        // of the Copernica library.
        myAmqp.Close(doFlush);
        
        // Benchmark if sending out one after another.
        if (sleepSeconds == 0) {
            auto elapsedMs = benchmarkStopwatch.GetElapsedMilliseconds();
            stringstream messageStr;
            messageStr << messageCount << " messages sent in: " << elapsedMs.count() << " ms";
            
            cout << messageStr.str() << endl;
        }
        
    } catch (exception& ex) {
        cerr << ex.what() << endl;
    }
    
    return 0;
}
