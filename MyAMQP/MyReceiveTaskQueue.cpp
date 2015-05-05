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

namespace MyAMQP {
    template <typename T>
    MyReceiveTaskQueue<T>::MyReceiveTaskQueue() :
    _mutex{},
    _conditional{},
    _breakWait{},
    _itemQueue{} {
        
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
        
        _conditional.notify_all();
    }

    template <typename T>
    void MyReceiveTaskQueue<T>::Reset() {
        _breakWait = false;
    }
    
    template <typename T>
    bool MyReceiveTaskQueue<T>::Empty() const {
        return _itemQueue.empty();
    }
    
    template <typename T>
    void MyReceiveTaskQueue<T>::Pop(T& item) {
        lock_guard<mutex> guard(_mutex);
        if (_itemQueue.empty()) {
            throw runtime_error("MyReceiveTaskQueue: Pop of empty queue");
        }
        
        item = move(_itemQueue.front());
        _itemQueue.pop();
    }


    // It is more usual to the method definitions inline in the header, but if we want them in the cpp like
    // the above, then explicit template instantiations are needed for all template specialisations.
    template class MyReceiveTaskQueue<packaged_task<int64_t(void)>>;
    template class MyReceiveTaskQueue<std::future<int64_t>>;
    template class MyReceiveTaskQueue<packaged_task<ssize_t(void)>>;
}