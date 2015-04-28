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
#include "MyStopwatch.h"
#include "../MyNetworkConnection/MyUnixNetworkConnection.h"

#include <thread>
#include <condition_variable>
#include <mutex>
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
            { "inline",   no_argument,            nullptr,          'i'},
            { "flush",    no_argument,            nullptr,          'F'},
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
        bool flushOnClose{};
        
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
                    
                case 'F':
                    flushOnClose = true;
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
        
        condition_variable benchmarkCondition{};
        mutex benchmarkMutex{};
        MyStopwatch benchmarkStopwatch{};
        
        int messageCount{};
        bool breakWait{};
        
        auto errorHandler = [&](string err) {
            cout << "Client error: " << err << endl;
            breakWait = true;
            benchmarkCondition.notify_one();            
        };
        
        MyAMQPClient myAmqp{move(netConnection)};
        
        auto completionHandlers = myAmqp.Open(loginInfo);
        completionHandlers.SubscribeToError(errorHandler);
        
        myAmqp.CreateHelloQueue(exchangeType, routingInfo);
        
        auto handler = [&](string const & message, int64_t tag, bool redelivered) {
            // Atomic recording of message count and elapsed time.
            lock_guard<mutex> guard(benchmarkMutex);
            auto isEndMessage = strcasecmp(message.c_str(), "end") == 0;
            
            if (!isEndMessage && !benchmarkStopwatch.IsRunning()) {
                benchmarkStopwatch.Start();
            }
            
            cout << message << ", tag: " << tag << ", redelivered: " << redelivered << endl;
            ++messageCount;
            
            if (isEndMessage && benchmarkStopwatch.IsRunning()) {
                benchmarkCondition.notify_one();
            }
            
            if (strcasecmp(message.c_str(), "goodbye") == 0) {
                breakWait = true;
                benchmarkCondition.notify_one();
            }
        };
        
        myAmqp.SubscribeToReceive(routingInfo.QueueName, handler, threaded);
        
        while (!breakWait) {
            unique_lock<mutex> lock(benchmarkMutex);
            benchmarkCondition.wait(lock, [&]{ return benchmarkStopwatch.IsRunning() || breakWait; });

            auto elapsedMs = benchmarkStopwatch.GetElapsedMilliseconds();
            stringstream messageStr;
            messageStr << messageCount << " messages received in: " << elapsedMs.count() << " ms";
            
            cout << messageStr.str() << endl;
            messageCount = 0;
            benchmarkStopwatch.Stop();
        }
        
        myAmqp.Close(flushOnClose);
        
    } catch (exception& ex) {
        cerr << ex.what() << endl;
    }
    
    return 0;
}
