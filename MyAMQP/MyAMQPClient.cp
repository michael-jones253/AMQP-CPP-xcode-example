/*
 *  MyAMQP.h
 *  MyAMQP
 *
 *  Created by Michael Jones on 17/04/2015.
 *  Copyright (c) 2015 Michael Jones. All rights reserved.
 *
 */

#include "MyAMQPClient.h"
#include "MyAMQPClientImpl.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <chrono>

using namespace AMQP;
using namespace std;
using namespace std::chrono;

namespace MyAMQP {
    
    void MyAMQPClient::CreateHelloQueue(ExchangeType exchangeType, MyAMQPRoutingInfo const& routingInfo) {
        _impl->CreateHelloQueue(exchangeType, routingInfo);
    }
    
    void MyAMQPClient::SendHelloWorld(string const& exchange, string const& key, string const& greeting) {
        _impl->SendHelloWorld(exchange, key, greeting);
    }
    
    void MyAMQPClient::SubscribeToReceive(string const& queueName,
                                          std::function<void(std::string const &, int64_t, bool)> const &handler,
                                          bool threaded) {
        _impl->SubscribeToReceive(queueName, handler, threaded);
    }
    
    MyAMQPClient::MyAMQPClient() :
    _impl{} {
        
    }
    
    MyAMQPClient::MyAMQPClient(std::unique_ptr<MyNetworkConnection> networkConnection) :
    _impl{}
    {
        _impl = unique_ptr<MyAMQPClientImpl>(new MyAMQPClientImpl(move(networkConnection)));
    }
    
    MyAMQPClient::MyAMQPClient(MyAMQPClient&& rhs) {        
        _impl = move(rhs._impl);
    }
    
    MyAMQPClient&  MyAMQPClient::operator=(MyAMQPClient&& rhs) {
        if (this != & rhs) {
            _impl = move(rhs._impl);
        }
        
        return *this;
    }
    
    MyAMQPClient::~MyAMQPClient() {
    }
    
    MyCompletionCallbacks MyAMQPClient::Open(MyLoginCredentials const& loginInfo) {
        return _impl->Open(loginInfo);
    }
    
    void MyAMQPClient::Close(bool flush) {
        _impl->Close(flush);
    }
    
    void MyAMQPClient::Pause() {
        _impl->Pause();
    }
    
    void MyAMQPClient::Resume() {
        _impl->Resume();
    }
}