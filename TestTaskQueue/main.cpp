//
//  main.cpp
//  TestTaskQueue
//
//  Created by Michael Jones on 22/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include <iostream>
#include "MyReceiveTaskQueue.h"
#include <assert.h>

using namespace std;
using namespace MyAMQP;

// template class std::packaged_task<int64_t(void)>;

namespace MyAMQP {
    // template class MyReceiveTaskQueue<packaged_task<int64_t(void)>>;
}

int main(int argc, const char * argv[]) {
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
 
    cout << "Goodbye world: " << endl;
    
    return 0;
}
