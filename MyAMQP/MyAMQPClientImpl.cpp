//
//  MyAMQPClientImpl.cpp
//  AMQP
//
//  Created by Michael Jones on 22/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyAMQPClientImpl.h"
#include "MyAMQPClient.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <chrono>

using namespace AMQP;
using namespace std;
using namespace std::chrono;

// Anonymous namespace.
namespace  {
    int64_t deliverMessage(MyAMQP::MyMessageCallback const &userHandler,
                           string const& message,
                           uint64_t tag,
                           bool redelivered) {
        
        // User handler may throw, in which case we don't return tag for acknowledgment.
        userHandler(message, tag, redelivered);
        
        return static_cast<int64_t>(tag);
    }
    
}

namespace MyAMQP {
    
    void MyAMQPClientImpl::CreateHelloQueue(ExchangeType exchangeType, MyAMQPRoutingInfo const& routingInfo) {
        unique_lock<mutex> lock(_mutex);

        auto const timeout = seconds(5);
        
        // Prevent a race with queue creation before underlying channel is created.
        // Don't wait forever if channel callback never happens.
        _conditional.wait_for(lock,timeout , [this]() {
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
            // If the bind worked then we can assume that the above operations of exchange and queue declare worked.
            _queueReady = true;
            _conditional.notify_one();
        });
        
        // Prevent a race with sending messages before the queue is ready for use.
        cout << "waiting for queue" << endl;
        
        // Don't wait forever if queue not declared.
        auto status = _conditional.wait_for(lock, timeout, [this]()->bool { return _queueReady; });
        
        if (!status) {
            throw runtime_error("Queue bind timed out");
        }
    }
    
    void MyAMQPClientImpl::SendHelloWorld(string const& exchange, string const& key, string const& greeting) {
        cout << greeting << endl;
        auto ret = _channel->publish(exchange, key, greeting.c_str());
        
        if (!ret) {
            throw runtime_error("message publish failed");
        }
    }
    
    void MyAMQPClientImpl::SubscribeToReceive(string const& queue,
                                              MyMessageCallback const &userHandler,
                                              bool threaded) {
        
        auto messageHandler = threaded ? CreateThreadedMessageCallback(userHandler)
                                        : CreateInlineMessageCallback(userHandler);
        
        if (threaded) {
            auto ackChannel = bind(&MyAMQPClientImpl::AckMessage, this, placeholders::_1);
            _receiveTaskProcessor.Start();
            _ackProcessor.Start(ackChannel);
        }
        
        // Register callback with Copernica library.
        _channel->consume(queue).onReceived(messageHandler);
    }
    
    MyAMQPClientImpl::MyAMQPClientImpl(std::unique_ptr<MyNetworkConnection> networkConnection) :
    ConnectionHandler{},
    _amqpConnection{},
    _channel{},
    _bufferedConnection{move(networkConnection)},
    _mutex{},
    _conditional{},
    _channelOpen{},
    _channelInError{},
    _queueReady{},
    _receiveTaskProcessor{},
    _channelFinalized{}
    {
        auto parseCallback = bind(&MyAMQPClientImpl::OnNetworkRead, this, placeholders::_1, placeholders::_2);
        auto onErrorCallback = bind(&MyAMQPClientImpl::OnNetworkReadError, this, placeholders::_1);
        
        _bufferedConnection.SetCallbacks(parseCallback, onErrorCallback);
    }
    
    MyAMQPClientImpl::~MyAMQPClientImpl() {
        try {
            Close(false);
        }
        catch(exception& ex)
        {
            cerr << "Exception in MyAMQPClient destruction: " << ex.what() << endl;
        }
    }
    
    // Interface implementation of data from the client that is destined for the server.
    void MyAMQPClientImpl::onData(AMQP::Connection *connection, const char *buffer, size_t size) {
        // cout << "onData: " << size << endl;
        // for (unsigned i=0; i<size; i++) cout << (int)buffer[i] << " ";
        // cout << endl;
        
        _bufferedConnection.SendToServer(buffer, size);
    }
    
    void MyAMQPClientImpl::onError(AMQP::Connection *connection, const char *message) {
        cout << "AMQP error:" << message << endl;
        _bufferedConnection.Close();
    }
    
    void MyAMQPClientImpl::onConnected(AMQP::Connection *connection) {
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
    
    void MyAMQPClientImpl::onClosed(AMQP::Connection *connection) {
        // FIX ME. Anything to do, when is this called?
        cout << "MyAMQPClient onClosed" << endl;
    }
    
    void MyAMQPClientImpl::Open(MyLoginCredentials const& loginInfo) {
        // A common problem with components that need to be opened/closed or stopped/started is not being
        // robust to multiple opens/closes. The pattern where we always close before opening overcomes this.
        Close(false);
        
        _bufferedConnection.Open(loginInfo.HostIpAddress);
        
        _amqpConnection = unique_ptr<AMQP::Connection>(new AMQP::Connection(this,
                                                                            AMQP::Login(loginInfo.UserName,
                                                                                        loginInfo.Password), "/"));
        _amqpConnection->login();
        
    }
    
    void MyAMQPClientImpl::Close(bool flush) {
        // Close must be robust to multiple calls or called before open.
        
        _receiveTaskProcessor.Stop(flush);
        
        // Presumably flushing of acks needs to be done before the channel is closed.
        // Definitely before the connection is closed.
        _ackProcessor.Stop(flush);

        auto finalize = [&](){
            cout << "channel finalized" << endl;
            _channelFinalized = true;
            _conditional.notify_all();
        };
        
        if (_channel) {
            _channel->close().onFinalize(finalize);
            
            unique_lock<mutex> lock(_mutex);            
            _conditional.wait(lock, [&]() { return _channelFinalized; });
        }
        
        cout << "Client closing" << endl;
        
        if (_amqpConnection) {
            _amqpConnection->close();
        }
        
        // This will block until read loop exits.
        _bufferedConnection.Close();

        _channel.reset();
        _amqpConnection.reset();
    }
    
    size_t MyAMQPClientImpl::OnNetworkRead(char const* buf, int len) {
        auto parsedBytes = _amqpConnection->parse(buf, len);
        
        return parsedBytes;
    }
    
    void MyAMQPClientImpl::OnNetworkReadError(std::string const& errorStr){
        cout << "MyAMQPClient read error: " << errorStr << endl;
        _bufferedConnection.Close();
    }
    
    void MyAMQPClientImpl::AckMessage(int64_t deliveryTag) {
        _channel->ack(deliveryTag);
        // cout << "Acked tag: " << deliveryTag << endl;
    }
    
    MessageCallback MyAMQPClientImpl::CreateInlineMessageCallback(MyMessageCallback const& userHandler) {
        // Take a copy of handler.
        auto receiveHandler = [this,userHandler](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
            try {
                // The Copernica library invokes this callback in the context for the socket read thread.
                // So rather than block this thread we package the handler to be invoked by another task.
                // This model assumes that the invoking of the handler needs to be done serially.
                userHandler(message.message(), deliveryTag, redelivered);
                
                // Acks are done from the context of this callback, which means they could potentially block on a
                // network send.
                _channel->ack(deliveryTag);
                
            }
            catch(exception const& ex) {
                cerr << "Receive handler error: " << ex.what() << endl;
            }
            
        };

        return receiveHandler;
    }
    
    MessageCallback MyAMQPClientImpl::CreateThreadedMessageCallback(MyMessageCallback const& userHandler) {
        // Take a copy of handler.
        auto receiveHandler = [this,userHandler](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
            try {
                // Because messages will be cached in the queue, we need to take a copy.
                string messageCopy{message.message()};
                
                auto handleReceive = bind(deliverMessage, userHandler, move(messageCopy), deliveryTag, redelivered);
                
                auto deliveryTask = packaged_task<int64_t(void)>{ move(handleReceive) };
                
                auto tag = deliveryTask.get_future();
                
                // The Copernica library invokes this callback in the context for the socket read thread.
                // So rather than block this thread we package the handler to be invoked by another task.
                // This model assumes that the invoking of the handler needs to be serialised.
                _receiveTaskProcessor.Push(move(deliveryTask));
                
                // Acks are done in another thread, to avoid the user handler blocking on a network send.
                // This means that it is possible that handlers are invoked and their corresponding acks
                // never sent if this client crashes in between invoking a series of handers before the queued
                // ack tasks are sent. This will cause redelivery. However there is always a slight chance of
                // redilivery for messages that have been processed, but ack not sent e.g. in the case of ack
                // being lost on the network. Therefore user handlers do need to be resilient to duplicate messages.
                _ackProcessor.Push(move(tag));
            }
            catch(exception const& ex) {
                cerr << "Receive handler error: " << ex.what() << endl;
            }
            
        };
        
        return receiveHandler;
    }


}