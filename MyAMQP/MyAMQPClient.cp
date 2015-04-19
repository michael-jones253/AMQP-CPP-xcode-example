/*
 *  MyAMQP.h
 *  MyAMQP
 *
 *  Created by Michael Jones on 17/04/2015.
 *  Copyright (c) 2015 Michael Jones. All rights reserved.
 *
 */

#include "MyAMQPClient.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <chrono>


using namespace std;
using namespace std::chrono;

namespace MyAMQP {
    
    void MyAMQPClient::CreateHelloQueue() {
        unique_lock<mutex> lock(_mutex);
        
        // Prevent a race with queue creation before underlying channel is created.
        _conditional.wait(lock, [this]() {
            
            // Review: check assumption that copernica will either callback on success or fail of channel open.
            return _channelOpen || _channelInError;
        });
        
        if (_channelInError) {
            throw runtime_error("Hello Channel not created");
        }
        
        // According to Copernica messages are cached, so for this example we assume queue declaration,
        // exchange declaration and bind can be sent one after another.
        
        // we declare a queue, an exchange and we publish a message
        _channel->declareQueue("my_queue").onSuccess([this]() {
            std::cout << "queue declared" << std::endl;
        });
        
        // declare an exchange
        _channel->declareExchange("my_exchange", AMQP::direct).onSuccess([]() {
            std::cout << "exchange declared" << std::endl;
        });
        
        // bind queue and exchange
        _channel->bindQueue("my_exchange", "my_queue", "key").onSuccess([this]() {
            std::cout << "queue bound to exchange" << std::endl;
            _queueReady = true;
            _conditional.notify_one();
        });
        
        // Prevent a race with sending messages before the queue is ready for use.
        cout << "waiting for queue" << endl;
        auto timeout = seconds(5);

        // Don't wait forever if queue not declared.
        auto status = _conditional.wait_for(lock, timeout, [this]() { return _queueReady; });
        
        if (!status) {
            throw runtime_error("Queue bind timed out");
        }
    }
    
    void MyAMQPClient::SendHelloWorld(const char *greeting) {
        cout << greeting << endl;
        auto ret = _channel->publish("my_exchange", "key", greeting);
        
        if (!ret) {
            throw runtime_error("message publish failed");
        }
    }
    
    MyAMQPClient::MyAMQPClient(std::unique_ptr<MyAMQPNetworkConnection> networkConnection) :
    _amqpConnection{},
    _networkConnection{},
    _channel{},
    _mutex{},
    _conditional{},
    _channelOpen{},
    _channelInError{},
    _queueReady{} {
        _networkConnection = move(networkConnection);
        
    }
    
    MyAMQPClient::~MyAMQPClient() {
        try {
            Close();
        }
        catch(exception& ex)
        {
            cerr << "Exception in MyAMQPClient destruction: " << ex.what() << endl;
        }
    }
    
    
    void MyAMQPClient::onData(AMQP::Connection *connection, const char *buffer, size_t size) {
        cout << "onData: " << size << endl;
        
        for (unsigned i=0; i<size; i++) cout << (int)buffer[i] << " ";
        cout << endl;
        
        auto bytesWritten = _networkConnection->SendToServer(buffer, size);
        
        cout << " FIX ME loop needed, written: " << bytesWritten << endl;
    }
    
    void MyAMQPClient::onError(AMQP::Connection *connection, const char *message) {
        cout << "AMQP error:" << message << endl;
        _networkConnection->Close();
    }
    
    void MyAMQPClient::onConnected(AMQP::Connection *connection) {
        cout << "AMQP login success" << endl;
        
        auto onChannelOpen = [this]() {
            cout << "Channel open" << endl;
            _channelOpen = true;
            _conditional.notify_one();
        };
        
        auto onChannelError = [this](char const* errMsg) {
            cout << "Channel error: " << errMsg << endl;
            _channelOpen = false;
            _channelInError = true;
            _channel->close();
        };
        
        // create channel if it does not yet exist
        if (!_channel) {
            _channel = unique_ptr<AMQP::Channel>(new AMQP::Channel(connection));
            
            // Set callbacks.
            _channel->onReady(onChannelOpen);
            _channel->onError(onChannelError);
        }
    }
    
    void MyAMQPClient::onClosed(AMQP::Connection *connection) {
    }
    
    void MyAMQPClient::Open(const string& ipAddress) {
        _networkConnection->Open(ipAddress,
                                 bind(&MyAMQPClient::OnNetworkRead, this, placeholders::_1, placeholders::_2),
                                 bind(&MyAMQPClient::OnNetworkReadError, this, placeholders::_1));
        
        _amqpConnection = unique_ptr<AMQP::Connection>(new AMQP::Connection(this, AMQP::Login("guest", "guest"), "/"));
        _amqpConnection->login();
        
    }
    
    void MyAMQPClient::Close() {
        _amqpConnection->close();
        _networkConnection->Close();
    }
    
    size_t MyAMQPClient::OnNetworkRead(char const* buf, int len) {
        auto parsedBytes = _amqpConnection->parse(buf, len);
        
        return parsedBytes;
    }
    
    void MyAMQPClient::OnNetworkReadError(std::string const& errorStr){
        cout << "MyAMQPClient read error: " << errorStr << endl;
        _networkConnection->Close();
    }
}