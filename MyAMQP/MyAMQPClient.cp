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
        
        /*
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
        _channel->declareQueue(routingInfo.QueueName).onSuccess([this]() {
            std::cout << "queue declared" << std::endl;
        });
        
        // declare an exchange
        _channel->declareExchange(routingInfo.ExchangeName, exchangeType).onSuccess([]() {
            std::cout << "exchange declared" << std::endl;
        });
        
        // bind queue and exchange
        _channel->bindQueue(routingInfo.ExchangeName, routingInfo.QueueName, routingInfo.Key).onSuccess([this]() {
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
         */
    }
    
    void MyAMQPClient::SendHelloWorld(string const& exchange, string const& key, string const& greeting) {
        _impl->SendHelloWorld(exchange, key, greeting);
        /*
        cout << greeting << endl;
        auto ret = _channel->publish(exchange, key, greeting.c_str());
        
        if (!ret) {
            throw runtime_error("message publish failed");
        }
        */
    }
    
    void MyAMQPClient::SubscribeToReceive(string const& queueName,
                                          function<void(string const &, bool)> const &handler) {
        _impl->SubscribeToReceive(queueName, handler);
        /*
        >(<#container_type &&__c#>), <#const std::function<void (const AMQP::Message &, bool)> &handler#>
        
        // Take a copy of handler.
        auto receiveHandler = [this,handler](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
            try {
                handler(message, redelivered);

                // Only ack if handler didn't throw.
                _channel->ack(deliveryTag);
            }
            catch(exception const& ex) {
                cerr << "Receive handler error: " << ex.what() << endl;
            }
            
        };

        _channel->consume(queue).onReceived(receiveHandler);
         */
    }
    
    MyAMQPClient::MyAMQPClient(std::unique_ptr<MyAMQPNetworkConnection> networkConnection) :
    _impl{}
    /*
    _amqpConnection{},
    _networkConnection{},
    _channel{},
    _mutex{},
    _conditional{},
    _channelOpen{},
    _channelInError{},
    _queueReady{}
     */
    {
        _impl = unique_ptr<MyAMQPClientImpl>(new MyAMQPClientImpl(move(networkConnection)));
    }
    
    MyAMQPClient::~MyAMQPClient() {
        /*
        try {
            Close();
        }
        catch(exception& ex)
        {
            cerr << "Exception in MyAMQPClient destruction: " << ex.what() << endl;
        }
         */
    }
    /*
    
    void MyAMQPClient::onData(AMQP::Connection *connection, const char *buffer, size_t size) {
        _impl->onData(connection, buffer, size);
        
        // cout << "onData: " << size << endl;
        // for (unsigned i=0; i<size; i++) cout << (int)buffer[i] << " ";
        // cout << endl;
        
        _networkConnection->SendToServer(buffer, size);
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
     */
    
    void MyAMQPClient::Open(MyLoginCredentials const& loginInfo) {
        _impl->Open(loginInfo);
        /*
        _networkConnection->Open(loginInfo.HostIpAddress,
                                 bind(&MyAMQPClient::OnNetworkRead, this, placeholders::_1, placeholders::_2),
                                 bind(&MyAMQPClient::OnNetworkReadError, this, placeholders::_1));
        
        _amqpConnection = unique_ptr<AMQP::Connection>(new AMQP::Connection(this,
                                                                            AMQP::Login(loginInfo.UserName,
                                                                            loginInfo.Password), "/"));
        _amqpConnection->login();
         */
        
    }
    
    void MyAMQPClient::Close() {
        _impl->Close();
        /*
        if (_amqpConnection) {
            _amqpConnection->close();
        }
        
        if (_networkConnection) {
            _networkConnection->Close();
        }
         */
    }
    /*
    
    size_t MyAMQPClient::OnNetworkRead(char const* buf, int len) {
        auto parsedBytes = _amqpConnection->parse(buf, len);
        
        return parsedBytes;
    }
    
    void MyAMQPClient::OnNetworkReadError(std::string const& errorStr){
        cout << "MyAMQPClient read error: " << errorStr << endl;
        _networkConnection->Close();
    }
     */
}