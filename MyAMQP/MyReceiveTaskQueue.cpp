//
//  MyReceiveTaskQueue.cpp
//  AMQP
//
//  Created by Michael Jones on 22/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyReceiveTaskQueue.h"
#include <future>

using namespace std;

template class std::packaged_task<int64_t(void)>;

namespace MyAMQP {
    template <typename T>
    MyReceiveTaskQueue<T>::MyReceiveTaskQueue() : _itemQueue{} {
        
    }
    
    template <typename T>
    void MyReceiveTaskQueue<T>::Push(T&& item) {
        {
            lock_guard<mutex> guard(_mutex);
            _itemQueue.push(move(item));
        }
        
        _conditional.notify_one();
    }

    
    template <typename T>
    bool MyReceiveTaskQueue<T>::Wait(T& item) {
        unique_lock<mutex> lock(_mutex);
        
        _conditional.wait(lock, [this]() { return !_itemQueue.empty() || _breakWait; }) ;
        
        if (_breakWait) {
            return false;
        }

        item = move(_itemQueue.front());
        _itemQueue.pop();
        
        return true;
    }
    
    template <typename T>
    void MyReceiveTaskQueue<T>::BreakWait() {
        _breakWait = true;
        
        _conditional.notify_one();
    }
    
    
    template class MyReceiveTaskQueue<int>;
    template class MyReceiveTaskQueue<packaged_task<int64_t(void)>>;
}