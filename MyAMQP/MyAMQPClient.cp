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

using namespace AMQP;
using namespace std;
using namespace std::chrono;

namespace MyAMQP {
    
    void MyAMQPClient::CreateHelloQueue(ExchangeType exchangeType) {
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
        _channel->declareExchange("my_exchange", exchangeType).onSuccess([]() {
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
        auto const timeout = seconds(5);

        // Don't wait forever if queue not declared.
        auto status = _conditional.wait_for(lock, timeout, [this]()->bool { return _queueReady; });
        
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
    ConnectionHandler{},
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
        // cout << "onData: " << size << endl;
        // for (unsigned i=0; i<size; i++) cout << (int)buffer[i] << " ";
        // cout << endl;
        
        auto amountWritten = size;
        
        while (amountWritten > 0) {
            auto bytesWritten = _networkConnection->SendToServer(buffer, size);
            amountWritten -= bytesWritten;
        }        
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
            
            // No need to notify under lock.
            _conditional.notify_one();
        };
        
        auto onChannelError = [this](char const* errMsg) {
            cout << "Channel error: " << errMsg << endl;
            _channelOpen = false;
            _channelInError = true;

            // No need to notify under lock.
            _conditional.notify_one();
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
        // FIX ME. Anything to do, when is this called?
        cout << "MyAMQPClient onClosed" << endl;
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