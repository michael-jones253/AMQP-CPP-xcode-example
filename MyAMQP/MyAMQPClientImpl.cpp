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
    // The Copernica library does not guarantee delivery of completion and error callbacks, so we protect against
    // waiting forever for these to occur.
    auto const CopernicaCompletionTimeout = seconds(5);
}

namespace MyAMQP {
    
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
    _threadedReceive{},
    _simulateAckDelay{},
    _receiveTaskProcessor{},
    _ackProcessor{},
    _requestProcessor{},
    _channelFinalized{},
    _completionNotifier{}
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
    
    void MyAMQPClientImpl::CreateHelloQueue(ExchangeType exchangeType, MyAMQPRoutingInfo const& routingInfo) {
        unique_lock<mutex> lock(_mutex);
        
        // Prevent a race with queue creation before underlying channel is created.
        // Don't wait forever if channel callback never happens.
        auto ok = _conditional.wait_for(lock, CopernicaCompletionTimeout , [this]() {
            return _channelOpen || _channelInError;
        });
        
        if (!ok) {
            throw runtime_error("Channel open timed out");
        }
        
        if (_channelInError) {
            throw runtime_error("Hello Channel not created");
        }
        
        auto onSuccess = [this]() {
            std::cout << "queue bound to exchange" << std::endl;
            {
                lock_guard<mutex> guard(_mutex);
                _queueReady = true;
            }
            _conditional.notify_one();
        };
        
        
        BindAmqpQueueToExchange(exchangeType, routingInfo, onSuccess);
        
        // Prevent a race with sending messages before the queue is ready for use.
        cout << "waiting for queue" << endl;
        
        // Don't wait forever if queue not declared.
        auto status = _conditional.wait_for(lock, CopernicaCompletionTimeout, [this]()->bool { return _queueReady; });
        
        if (!status) {
            throw runtime_error("Queue bind timed out");
        }
        
    }
    
    void MyAMQPClientImpl::SendHelloWorld(string const& exchange, string const& key, string const& greeting) {
        auto publish = [this, &exchange, &key, &greeting]()->MyRequestResult {
            cout << greeting << endl;
            auto ok = _channel->publish(exchange, key, greeting.c_str());
            return MyRequestResult{ ok };
        };
        
        auto publishTask = packaged_task<MyRequestResult(void)>{ publish };
        auto publishResult = publishTask.get_future();
        _requestProcessor.Push(move(publishTask));
        
        if (!publishResult.get().Ok) {
            throw runtime_error("message publish failed");
        }
    }
    
    void MyAMQPClientImpl::SubscribeToReceive(string const& queue,
                                              MyMessageCallback const &userHandler,
                                              bool threaded) {
        
        auto messageHandler = threaded ? CreateThreadedMessageCallback(userHandler)
        : CreateInlineMessageCallback(userHandler);
        
        _threadedReceive = threaded;
        
        if (threaded) {
            auto ackChannel = bind(&MyAMQPClientImpl::AckMessage, this, placeholders::_1);
            _receiveTaskProcessor.Start();
            _ackProcessor.Start(ackChannel);
        }
        
        // Register callback with Copernica library.
        auto registerReceiveHandler = [this, &queue, &messageHandler]()->MyRequestResult {
            // Copernica library takes a copy of the handler so ok to pass a reference to a
            // temporary.
            _channel->consume(queue).onReceived(messageHandler);
            
            // Assume it worked.
            return MyRequestResult{ true };
        };
        
        auto registerReceiveTask = packaged_task<MyRequestResult(void)>{ registerReceiveHandler };
        auto registerResult = registerReceiveTask.get_future();
        _requestProcessor.Push(move(registerReceiveTask));
        auto result = registerResult.get();
        
        // We don't care about the result, but do the get to make this synchronous.
        (void)result;
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
            {
                lock_guard<mutex> guard(_mutex);
                _channelOpen = true;
            }
            
            // No need to notify under lock.
            _conditional.notify_one();
        };
        
        auto onChannelError = [this](char const* errMsg) {
            cout << "Channel error: " << errMsg << endl;
            {
                lock_guard<mutex> guard(_mutex);
                _channelOpen = false;
                _channelInError = true;
            }
            
            // No need to notify under lock.
            _conditional.notify_one();
        };
        
        // create channel if it does not yet exist
        if (!_channel) {
            _channel = make_unique<AMQP::Channel>(connection);
            
            // Set callbacks.
            _channel->onReady(onChannelOpen);
            _channel->onError(onChannelError);
        }
    }
    
    void MyAMQPClientImpl::onClosed(AMQP::Connection *connection) {
        // FIX ME. Anything to do, when is this called?
        cout << "MyAMQPClient onClosed" << endl;
    }
    
    MyCompletionCallbacks MyAMQPClientImpl::Open(MyLoginCredentials const& loginInfo) {
        // A common problem with components that need to be opened/closed or stopped/started is not being
        // robust to multiple opens/closes. The pattern where we always close before opening overcomes this.
        Close(false);
        
        // Reset state information to allow for reopening.
        {
            lock_guard<mutex> guard(_mutex);
            _channelOpen = false;
            _channelInError = false;
            _queueReady = false;
            _channelFinalized = false;
        }
        
        _bufferedConnection.Open(loginInfo.HostIpAddress);
        
        _amqpConnection = make_unique<AMQP::Connection>(this,
                                                        AMQP::Login(loginInfo.UserName, loginInfo.Password), "/");
        _amqpConnection->login();
        
        _requestProcessor.Start();
        
        return _completionNotifier.CreateCompletionCallbacks();
    }
    
    void MyAMQPClientImpl::Close(bool flush) {
        // Close must be robust to multiple calls or called before open.
        
        int retCode{};
        
        if (flush) {
            // All tasks must be executed first before waiting on the futures.
            _receiveTaskProcessor.Stop(flush);
            
            // Presumably flushing of acks needs to be done before the channel is closed.
            // Definitely before the connection is closed.
            _ackProcessor.Stop(flush);
        }
        else {
            // Immediate stop means don't wait for tasks to finish.
            _ackProcessor.Stop(flush);
            _receiveTaskProcessor.Stop(flush);
        }
        
        try {
            CloseAmqpChannel(flush);
            CloseAmqpConnection();
        } catch (exception const& ex) {
            cerr << "AMQP close error: " << ex.what() << endl;
            retCode = -1;
        }
        
        // This will block until read loop exits.
        _bufferedConnection.Close();
        _completionNotifier.NotifyExit(retCode);
        
        _channel.reset();
        _amqpConnection.reset();
        
        _requestProcessor.Stop();
    }
    
    void MyAMQPClientImpl::Pause() {
        _pauseClient = true;
        _bufferedConnection.PauseReadLoop();
        _ackProcessor.Stop(false);
        _receiveTaskProcessor.Stop(false);
    }
    
    void MyAMQPClientImpl::Resume() {
        if (!_pauseClient) {
            return;
        }
        
        if (_threadedReceive) {
            _ackProcessor.Resume();
            _receiveTaskProcessor.Start();
        }
        
        _bufferedConnection.ResumeReadLoop();
    }
    
    void MyAMQPClientImpl::SimulateAckDelay(bool delay) {
        _simulateAckDelay = delay;
    }
    
    size_t MyAMQPClientImpl::OnNetworkRead(char const* buf, int len) {
        // Called from the context of the network read thread so parse requests and
        // other requests such as close on the Copernica API are serialised through
        // the "request processor".
        auto parseTask = packaged_task<MyRequestResult(void)>{ [this, buf, len]() -> MyRequestResult {
            auto parsedBytes = _amqpConnection->parse(buf, len);
            return MyRequestResult{ static_cast<ssize_t>(parsedBytes) };
        }};
        
        auto parseResult = parseTask.get_future();
        
        _requestProcessor.Push(move(parseTask));
        
        auto parsedBytes = parseResult.get();
        
        return parsedBytes.ByteCount;
    }
    
    void MyAMQPClientImpl::OnNetworkReadError(std::string const& errorStr){
        string err = "MyAMQPClient read error: " + errorStr;
        _completionNotifier.NotifyError(err);
    }
    
    
    int64_t MyAMQPClientImpl::DeliverMessage(MyAMQP::MyMessageCallback const &userHandler,
                                             string const& message,
                                             uint64_t tag,
                                             bool redelivered) {
        
        // User handler may throw, in which case we don't return tag for acknowledgment.
        userHandler(message, tag, redelivered);
        
        return static_cast<int64_t>(tag);
    }
    
    void MyAMQPClientImpl::AckMessage(int64_t deliveryTag) {
        
        if (_simulateAckDelay) {
            this_thread::sleep_for(milliseconds(1));
        }
        
        auto ackTask = packaged_task<MyRequestResult(void)> {[this, deliveryTag]() ->MyRequestResult{
            auto acked = _channel->ack(deliveryTag);
            return MyRequestResult{ acked };
        }};
        
        auto ackResult = ackTask.get_future();
        _requestProcessor.Push(move(ackTask));
        auto result = ackResult.get();
        
        // We don't care about the value of the result, but we do a get to capture any exceptions
        // on the network send. It also make this call synchronous which is the effect we want when
        // flushing acks during shutdown.
        (void)result;
    }
    
    void MyAMQPClientImpl::BindAmqpQueueToExchange(ExchangeType exchangeType,
                                                   MyAMQPRoutingInfo const& routingInfo,
                                                   function<void(void)> const& onSuccess) {
        
        // According to Copernica example messages are cached. So for this example we assume queue declaration,
        // exchange declaration and bind can be sent one after another without waiting for the completion callbacks.
        auto bind = [this, exchangeType, &routingInfo, &onSuccess]()->MyRequestResult {
            
            // we declare a queue, an exchange and we publish a message
            _channel->declareQueue(routingInfo.QueueName).onSuccess([this]() {
                std::cout << "queue declared" << std::endl;
            });
            
            // declare an exchange
            _channel->declareExchange(routingInfo.ExchangeName, exchangeType).onSuccess([]() {
                std::cout << "exchange declared" << std::endl;
            });
            
            // bind queue and exchange
            _channel->bindQueue(routingInfo.ExchangeName, routingInfo.QueueName, routingInfo.Key).onSuccess(onSuccess);
            return MyRequestResult{ true };
        };
        
        // The Copernica API is not thread safe and queue operations from the main application thread can contend with
        // parse requests from the network thread. So we serialise via the request processor.
        // Prior to serialising via the "request processor" I have not seen a problem during this opening sequence
        // however I have seen a crash under load during the close down. However, because of the potential we should fix
        // it every where.
        auto bindTask = packaged_task<MyRequestResult(void)>{ bind };
        auto bindResult = bindTask.get_future();
        auto accepted = _requestProcessor.Push(move(bindTask));
        
        if (accepted) {
            auto result = bindResult.get();
            // We don't care about the result, because success is dependent on invoking the onSuccess callback,
            // within the timeout limit.
            (void)result;
        }
    }
    
    void MyAMQPClientImpl::CloseAmqpChannel(bool flush) {
        auto finalize = [&](){
            cout << "channel finalized" << endl;
            {
                // Consistent coding standard, but mutex not necessary for single bool that is not being tested.
                lock_guard<mutex> guard(_mutex);
                _channelFinalized = true;
            }
            _conditional.notify_all();
        };
        
        auto closeChannel = [this, &finalize]() {
            _channel->close().onFinalize(finalize);
            return MyRequestResult{ true };
        };
        
        if (_channel && flush) {
            auto closeTask = packaged_task<MyRequestResult(void)> { closeChannel };
            // API Calls to the Copernica library are not thread safe, so serialize throught the request processor.
            auto accepted = _requestProcessor.Push(move(closeTask));
            
            if (accepted) {
                // We don't care about the result future of the above task, because success is dependent on receiving
                // the finalize callback.
                unique_lock<mutex> lock(_mutex);
                auto ok = _conditional.wait_for(lock, CopernicaCompletionTimeout, [&]() { return _channelFinalized; });
                if (!ok) {
                    throw runtime_error("Channel finalize timed out");
                }
            }
        }
    }
    
    void MyAMQPClientImpl::CloseAmqpConnection() {
        auto closeConnection = [this]() -> MyRequestResult {
            bool ok{};
            cout << "Client closing" << endl;
            if (_amqpConnection) {
                ok = _amqpConnection->close();
            }
            
            return MyRequestResult{ ok };
        };
        
        // API Calls to the Copernica library are not thread safe, so serialize throught the request processor.
        auto closeTask = packaged_task<MyRequestResult(void)>{ closeConnection  };
        auto closeResult = closeTask.get_future();
        auto accepted = _requestProcessor.Push(move(closeTask));
        
        if (accepted) {
            auto retCode = closeResult.get();
            
            // We don't care about the result, but we try and get it to capture any exception that was thrown.
            (void)retCode;
        }
        
    }
    
    MessageCallback MyAMQPClientImpl::CreateInlineMessageCallback(MyMessageCallback const& userHandler) {
        // Take a copy of handler.
        auto receiveHandler = [this,userHandler](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
            try {
                // The Copernica library invokes this callback in the context for the socket read thread.
                // So rather than block this thread we package the handler to be invoked by another task.
                // This model assumes that the invoking of the handler needs to be done serially.
                userHandler(message.message(), deliveryTag, redelivered);
                
                if (_simulateAckDelay) {
                    this_thread::sleep_for(milliseconds(1));
                }
                
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
                
                auto handleReceive = bind(
                                          &MyAMQPClientImpl::DeliverMessage,
                                          this,
                                          userHandler,
                                          move(messageCopy),
                                          deliveryTag,
                                          redelivered);
                
                auto deliveryTask = packaged_task<int64_t(void)>{ move(handleReceive) };
                
                auto tag = deliveryTask.get_future();
                
                // The Copernica library invokes this callback in the context for the socket read thread.
                // So rather than block this thread we package the handler to be invoked by another task.
                // This model assumes that the invoking of the handler needs to be serialised.
                auto accepted = _receiveTaskProcessor.Push(move(deliveryTask));
                
                if (!accepted) {
                    return;
                }
                
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