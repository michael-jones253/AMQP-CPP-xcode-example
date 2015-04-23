//
//  main.cpp
//  TestTaskQueue
//
//  Created by Michael Jones on 22/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyReceiveTaskQueue.h"
#include "MyTaskProcessor.h"
#include "MyAckProcessor.h"
#include <assert.h>
#include <iostream>
#include <chrono>
#include <vector>

using namespace MyAMQP;
using namespace std;
using namespace std::chrono;
using namespace std::this_thread;


int main(int argc, const char * argv[]) {
    try {
        // insert code here...
        MyReceiveTaskQueue<packaged_task<int64_t(void)>> taskQueue{};
        MyReceiveTaskQueue<int> xtaskQueue{};
        
        auto task = packaged_task<int64_t(void)>{ []() { return 99; }};
        
        auto ret = task.get_future();
        taskQueue.Push(move(task));
        
        packaged_task<int64_t(void)> poppedTask;
        
        auto ok = taskQueue.Wait(poppedTask);
        
        if (ok) {
            poppedTask();
        }
        
        auto tag = ret.get();
        
        assert(tag == 99);
        
        cout << "Hello, World: !" << tag << endl;
        
        taskQueue.BreakWait();
        
        auto endTask = taskQueue.Wait(poppedTask);
        
        if (endTask) {
            // This should never execute and if it does it will crash.
            poppedTask();
        }
        
        MyTaskProcessor processor;
        
        processor.Start();
        
        
        vector<future<int64_t>> results{};
        
        for (int x = 0; x < 100; x++) {
            auto task = packaged_task<int64_t(void)>{ [=]() { return x; }};
            
            results.push_back(move(task.get_future()));
            
            processor.Push(move(task));
        }
        
        for (auto& result : results) {
            try {
                auto res = result.get();
                
                cout << res << endl;
            }
            catch(exception const& ex) {
                cout << "result get: " << ex.what() << endl;
            }
        }
        
        results.clear();
        
        processor.Stop();
        
        // processor.Start();
        
        MyAckProcessor ackProcessor{};
        MyTaskProcessor anotherProcessor;
        
        ackProcessor.Start();
        anotherProcessor.Start();
        
        for (int x = 0; x < 100; x++) {
            auto task = packaged_task<int64_t(void)>{ [=]() { sleep_for(seconds(1));  return x; }};
            
            auto ackFuture = task.get_future();
            
            ackProcessor.Push(move(ackFuture));
            // results.push_back(move(ackFuture));

            
            anotherProcessor.Push(move(task));
        }
        
        sleep_for(seconds(2));
        for (auto& result : results) {
            try {
                auto res = result.get();
                
                cout << res << endl;
            }
            catch(exception const& ex) {
                cout << "result get: " << ex.what() << endl;
            }
        }

        
        while (true) {
            sleep_for(seconds(2));
        }
        cout << "Goodbye world: " << endl;
    }
    catch (exception const& ex) {
        cerr << ex.what() << endl;
    }
    return 0;
}
