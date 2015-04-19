//
//  main.cpp
//  Sender
//
//  Created by Michael Jones on 17/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include <iostream>
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
            { "fanout", no_argument, nullptr, 'f'},
            { "count",   required_argument,      nullptr,           'c' },
            { "sleep",   required_argument,      nullptr,           's' },
            { NULL,    0,                 nullptr,           0 }
        };
        
        bool fanOut{};
        int messageCount{100};
        int sleepSeconds{};
        
        do {
            auto ch = getopt_long(argc, (char* const *)(argv), "fc:s:", longopts, nullptr);
            
            switch (ch) {
                case 'f':
                    fanOut = true;
                    break;

                case 'c':
                    messageCount = stoi(optarg);
                    break;
                    
                case 's':
                    sleepSeconds = stoi(optarg);
                    
                case -1:
                    break;
                    
                default:
                    throw runtime_error("Command line error");
                    break;
            }
            
            if (ch == -1) {
                break;
            }
            
        } while (true);


        auto netConnection = unique_ptr<MyNetworkConnection>(new MyNetworkConnection());
        
        MyAMQPClient myAmqp{move(netConnection)};

        myAmqp.Open("127.0.0.1");
        
        myAmqp.CreateHelloQueue();
        
        for (int count = 0; count < messageCount; count++) {
            myAmqp.SendHelloWorld("sawadee krup");
            
            sleep_for(seconds(sleepSeconds));
        }
        
        myAmqp.Close();
        
    } catch (exception& ex) {
        cerr << ex.what() << endl;
    }
    
    return 0;
}
