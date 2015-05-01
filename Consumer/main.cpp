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
#include "MySignalCallbacks.h"
#include "MyKeyboardInput.h"
#include "MyUnixNetworkConnection.h"

#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <getopt.h>
#include <assert.h>

using namespace MyAMQP;
using namespace MyUtilities;
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

void OpenQueueForReceive(MyAMQPClient& client,
                         AMQP::ExchangeType exchangeType,
                         MyAMQPRoutingInfo const & routingInfo,
                         bool threaded,
                         MyMessageCallback const& messageCallback) {
    
    client.CreateHelloQueue(exchangeType, routingInfo);
    
    client.SubscribeToReceive(routingInfo.QueueName, messageCallback, threaded);
}

int main(int argc, const char * argv[]) {
    // insert code here...
    try {
        /* options descriptor */
        static struct option longopts[] = {
            { "fanout",  no_argument,            nullptr,           'f'},
            { "topic",   no_argument,            nullptr,           't'},
            { "inline",   no_argument,            nullptr,          'i'},
            { "noflush",    no_argument,            nullptr,        'N'},
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
        bool flushOnClose{true};
        
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
                    
                case 'N':
                    flushOnClose = false;
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
        bool isEndMessage{};
        bool caughtTerminate{};
        bool caughtReload{};
        
        auto termHandler = [&](bool x, bool y) {
            cout << "SIGTERM." << endl;
            {
                // FIX ME - wait on terminate predicate.
                lock_guard<mutex> guard(benchmarkMutex);
                caughtTerminate = true;
            }
            
            benchmarkCondition.notify_one();
            
        };
        
        auto hupHandler = [&](bool, bool) {
            cout << "SIGHUP - reopening." << endl;
            {
                lock_guard<mutex> guard(benchmarkMutex);
                caughtReload = true;
            }
            
            benchmarkCondition.notify_one();
        };
        
        MyAMQPClient ampqClient{};
        
        auto ctrlCHandler = [&](bool, bool) {
            ampqClient.Pause();
            
            vector<string> options{"abort", "flush and quit", "continue"};
            auto input = MyKeyboardInput::GetOption(options);
            switch (input) {
                case 'a': {
                    lock_guard<mutex> guard(benchmarkMutex);
                    flushOnClose = false;
                    breakWait = true;
                }
                benchmarkCondition.notify_one();
                    break;
                    
                case 'f': {
                    lock_guard<mutex> guard(benchmarkMutex);
                    flushOnClose = true;
                    breakWait = true;
                }
                benchmarkCondition.notify_one();
                    break;
                    
                default:
                    break;
            }
            
            ampqClient.Resume();
            
        };
        
        auto pipeHandler = [](bool, bool) {
            // Without this a broken pipe e.g. write to a socket which has been closed by the server will crash
            // this program. However this program catches it and lets the client act on the resulting network
            // exception.
            cerr << "Handling broken PIPE" << endl;
        };
        
        // Singletons can make it very difficult for the coder to determine destruction order of globals,
        // sometimes with unexpected results. See RAII signal ownership below to add some clear determination
        // to signal handling release.
        
        MySignalHandler::Instance()->Initialise(false);
        
        // RAII release of signal handlers from OS.
        MySignalCallbacks signalCallbacks;
        
        signalCallbacks.InstallTerminateHandler(termHandler);
        signalCallbacks.InstallReloadHandler(hupHandler);
        signalCallbacks.InstallCtrlCHandler(ctrlCHandler);
        signalCallbacks.InstallBrokenPipeHandler(pipeHandler);
        
        auto errorHandler = [&](string const& err) {
            cout << "Client error: " << err << endl;
            {
                lock_guard<mutex> guard(benchmarkMutex);
                breakWait = true;
            }
            benchmarkCondition.notify_one();
        };
        
        // Move a client with network connection in.
        ampqClient = MyAMQPClient{move(netConnection)};
        
        auto completionHandlers = ampqClient.Open(loginInfo);
        completionHandlers.SubscribeToError(errorHandler);
        
        auto messageHandler = [&](string const & message, int64_t tag, bool redelivered) {
            // Atomic recording of message count and elapsed time.
            {
                lock_guard<mutex> guard(benchmarkMutex);
                isEndMessage = strcasecmp(message.c_str(), "end") == 0;
                
                if (!isEndMessage && !benchmarkStopwatch.IsRunning()) {
                    benchmarkStopwatch.Start();
                }
                
                if (strcasecmp(message.c_str(), "goodbye") == 0) {
                    breakWait = true;
                }
                
                cout << message << ", tag: " << tag << ", redelivered: " << redelivered << endl;
                ++messageCount;
            }
            
            if (isEndMessage && benchmarkStopwatch.IsRunning()) {
                benchmarkCondition.notify_one();
            }
            
            if (breakWait) {
                benchmarkCondition.notify_one();
            }
            
        };
        
        OpenQueueForReceive(ampqClient, exchangeType, routingInfo, threaded, messageHandler);
        
        while (!breakWait) {
            unique_lock<mutex> lock(benchmarkMutex);
            benchmarkCondition.wait(lock, [&]{
                return (isEndMessage && benchmarkStopwatch.IsRunning())
                || breakWait
                || caughtReload; });
            
            auto elapsedMs = benchmarkStopwatch.GetElapsedMilliseconds();
            stringstream messageStr;
            messageStr << messageCount << " messages received in: " << elapsedMs.count() << " ms";
            
            cout << messageStr.str() << endl;
            messageCount = 0;
            isEndMessage = false;
            benchmarkStopwatch.Stop();
            
            if (caughtReload) {
                ampqClient.Close(flushOnClose);
                ampqClient.Open(loginInfo);
                OpenQueueForReceive(ampqClient, exchangeType, routingInfo, threaded, messageHandler);
                cout << "Reopened" << endl;
                caughtReload = false;
            }
        }
        
        ampqClient.Close(flushOnClose);
        
    } catch (exception& ex) {
        cerr << ex.what() << endl;
    }
    
    return 0;
}
