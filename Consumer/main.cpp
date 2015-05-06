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
            { "sleep",   required_argument,      nullptr,           's'},
            { "delay",   no_argument,     nullptr,                  'd'},
            { NULL,    0,                 nullptr,                    0}
        };
        
        MyLoginCredentials loginInfo{"127.0.0.1", "guest", "guest"};
        
        // Default exchange type.
        AMQP::ExchangeType exchangeType{AMQP::ExchangeType::direct};
        
        MyAMQPRoutingInfo routingInfo{"my_exchange", "key", "my_queue"};
        
        int sleepSeconds{};
        bool threaded{true};
        bool flushOnClose{true};
        bool messageProcessDelay{};
        
        auto const usageStr = string("Usage: [--fanout | --topic {default direct}] [--count <integer>] [--sleep <seconds>]");
        
        do {
            auto ch = getopt_long(argc, (char* const *)(argv), "ftie:k:q:c:s:d", longopts, nullptr);
            
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
                    
                case 'd':
                    messageProcessDelay = true;
                    
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
        bool resumeRequested{};
        
        auto termHandler = [&](bool x, bool y) {
            cout << "SIGTERM." << endl;
            // FIX ME - wait on terminate predicate.
            // NB Signal Trampoline means we must not lock the mutex.
            caughtTerminate = true;
            
            benchmarkCondition.notify_one();
            
        };
        
        auto hupHandler = [&](bool, bool) {
            cout << "SIGHUP - reopening." << endl;
            // NB Signal Trampoline means we must not lock the mutex.
            caughtReload = true;
            
            benchmarkCondition.notify_one();
        };
        
        MyAMQPClient ampqClient{};
        
        auto ctrlCHandler = [&](bool, bool) {
            ampqClient.Pause();
            
            vector<string> options{"terminate", "quit and flush", "ack delay", "message delay", "continue with no delay"};
            auto input = MyKeyboardInput::GetOption(options);
            switch (input) {
                case 't':
                    // NB Signal Trampoline means we must not lock the mutex.
                    flushOnClose = false;
                    breakWait = true;
                    break;
                    
                case 'q':
                    // NB Signal Trampoline means we must not lock the mutex.
                    flushOnClose = true;
                    breakWait = true;
                    break;
                    
                case 'a':
                    ampqClient.SimulateAckDelay(true);
                    break;
                    
                case 'm':
                    messageProcessDelay = true;
                    break;
                    
                case 'c':
                    ampqClient.SimulateAckDelay(false);
                    messageProcessDelay = false;
                    break;
                    
                default:
                    break;
            }
            
            // Because of sigtramp (Signal Trampoline) where the signalling line of execution parks itself
            // on some random part of the main thread restarting of threads is done in the main line of execution
            // on the main thread, so we signal without the lock here (in case lock was taken just before sigtramp).
            resumeRequested = true;
            benchmarkCondition.notify_one();
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
        
        // Move assignment of a client with network connection.
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
                
                ++messageCount;
                if (messageProcessDelay) {
                    sleep_for(milliseconds(1));
                }
                
                cout << message << ", tag: " << tag << ", redelivered: " << redelivered << endl;
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
            {
                // We have to be very careful with deadlock when more than one mutex is involved.
                unique_lock<mutex> lock(benchmarkMutex);
                benchmarkCondition.wait(lock, [&]{
                    return (isEndMessage && benchmarkStopwatch.IsRunning())
                    || breakWait
                    || resumeRequested
                    || caughtReload; });
                
                auto elapsedMs = benchmarkStopwatch.GetElapsedMilliseconds();
                stringstream messageStr;
                messageStr << messageCount << " messages received in: " << elapsedMs.count() << " ms";
                
                cout << messageStr.str() << endl;
                messageCount = 0;
                isEndMessage = false;
                benchmarkStopwatch.Stop();
            }
            
            if (resumeRequested) {
                cout << "Resuming" << endl;
                ampqClient.Resume();
                resumeRequested = false;
            }
            
            if (caughtReload) {
                cout << "Reopening" << endl;
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
