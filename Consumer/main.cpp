//
//  main.cpp
//  Consumer
//
//  Created by Michael Jones on 21/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//
#include <iostream>
#include <sstream>
#include <memory>
#include "MyAMQPClient.h"
#include "../MyNetworkConnection/MyNetworkConnection.h"
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
            { "exchange",     required_argument,            nullptr,'e'},
            { "key",     required_argument,            nullptr,     'k'},
            { "queue",     required_argument,            nullptr,   'q'},
            { "count",   required_argument,      nullptr,           'c' },
            { "sleep",   required_argument,      nullptr,           's' },
            { NULL,    0,                 nullptr,           0 }
        };
        
        MyLoginCredentials loginInfo{"127.0.0.1", "guest", "guest"};
        
        // Default exchange type.
        AMQP::ExchangeType exchangeType{AMQP::ExchangeType::direct};
        
        MyAMQPRoutingInfo routingInfo{"my_exchange", "key", "my_queue"};
        
        int messageCount{100};
        int sleepSeconds{};
        
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
        
        // To Do: args for server host, username and password.
        myAmqp.Open(loginInfo);
        
        myAmqp.CreateHelloQueue(exchangeType, routingInfo);
        myAmqp.SubscribeToReceive(routingInfo.QueueName);
        
        while (true) {
            // FIX ME - just temporary.
            sleep_for(seconds(3));
        }
        
        // FIX ME code never executed.
        myAmqp.Close();
        
    } catch (exception& ex) {
        cerr << ex.what() << endl;
    }
    
    return 0;
}
