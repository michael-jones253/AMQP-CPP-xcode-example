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
#include "../MyNetworkConnection/MyUnixNetworkConnection.h"
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
            { "inline",   no_argument,            nullptr,          'i'},
            { "exchange",     required_argument,            nullptr,'e'},
            { "key",     required_argument,            nullptr,     'k'},
            { "queue",     required_argument,            nullptr,   'q'},
            { "sleep",   required_argument,      nullptr,           's' },
            { NULL,    0,                 nullptr,           0 }
        };
        
        MyLoginCredentials loginInfo{"127.0.0.1", "guest", "guest"};
        
        // Default exchange type.
        AMQP::ExchangeType exchangeType{AMQP::ExchangeType::direct};
        
        MyAMQPRoutingInfo routingInfo{"my_exchange", "key", "my_queue"};
        
        int sleepSeconds{};
        bool threaded{true};
        
        auto const usageStr = string("Usage: [--fanout | --topic {default direct}] [--count <integer>] [--sleep <seconds>]");
        
        do {
            auto ch = getopt_long(argc, (char* const *)(argv), "ftie:k:q:c:s:", longopts, nullptr);
            
            switch (ch) {
                case 'f':
                    exchangeType = AMQP::ExchangeType::fanout;
                    break;
                    
                case 't':
                    // FIX ME.
                    throw runtime_error("Exchange type topic not implemented yet");
                    
                case 'i':
                    cout << "inline handler" << endl;
                    threaded = false;
                    break;
                    
                case 'e':
                    routingInfo.ExchangeName = optarg;
                    break;
                    
                case 'k':
                    routingInfo.Key = optarg;
                    break;
                    
                case 'q':
                    routingInfo.QueueName = optarg;
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
        
        bool shouldRun = true;
        
        time_point<system_clock> startTime{};
        time_point<system_clock> endTime{};
        bool benchmarkingStarted = false;
        bool benchmarkingEnded = false;
        
        int messageCount{};
        auto handler = [&](string const & message, int64_t tag, bool redelivered) {
            auto isEndMessage = strcasecmp(message.c_str(), "end") == 0;
            
            if (!isEndMessage && !benchmarkingStarted) {
                startTime = system_clock::now();
                benchmarkingStarted = true;
            }
            
            cout << message << ", tag: " << tag << ", redelivered: " << redelivered << endl;
            ++messageCount;
            
            if (isEndMessage && benchmarkingStarted) {
                benchmarkingEnded = true;
                endTime = system_clock::now();
            }
            
            if (strcasecmp(message.c_str(), "goodbye") == 0) {
                shouldRun = false;
            }
        };
        
        myAmqp.SubscribeToReceive(routingInfo.QueueName, handler, threaded);
        
        while (shouldRun) {
            // FIX ME - anything better to do?
            sleep_for(seconds(3));
            
            if (benchmarkingEnded) {
                auto elapsed = endTime - startTime;
                auto elapsedMs = duration_cast<milliseconds>(elapsed);
                stringstream messageStr;
                messageStr << messageCount << " messages received in: " << elapsedMs.count() << " ms";
                
                cout << messageStr.str() << endl;
                
                benchmarkingEnded = false;
                benchmarkingStarted = false;
                messageCount = 0;
            }
        }
        
        myAmqp.Close();
        
    } catch (exception& ex) {
        cerr << ex.what() << endl;
    }
    
    return 0;
}
