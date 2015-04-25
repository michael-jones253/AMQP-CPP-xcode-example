//
//  MyAMQPClientImpl.h
//  AMQP
//
//  Created by Michael Jones on 22/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef __AMQP__MyAMQPClientImpl__
#define __AMQP__MyAMQPClientImpl__

#include <stdio.h>
#include "amqpcpp.h"
#include "MyAMQPTypes.h"
#include "MyAMQPBufferedConnection.h"
#include "MyLoginCredentials.h"
#include "MyAMQPRoutingInfo.h"

#include "MyTaskProcessor.h"
#include "MyAckProcessor.h"

#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>

/* The class below is not exported */

namespace MyAMQP {
    
    // Hides the Copernica amqp includes from Client applications.
    class MyAMQPClientImpl final : public AMQP::ConnectionHandler {
        // Private constructor for use with friend class only.
        friend class MyAMQPClient;
        
        // Copernica open source stuff.
        std::unique_ptr<AMQP::Connection> _amqpConnection;
        
        // My stuff.
        std::unique_ptr<MyAMQPBufferedConnection> _networkConnection;
        
        // Open source stuff.
        std::unique_ptr<AMQP::Channel> _channel;
        
        std::mutex _mutex;
        
        std::condition_variable _conditional;
        
        std::atomic<bool> _channelOpen;
        
        std::atomic<bool> _channelInError;
        
        std::atomic<bool> _queueReady;
        
        MyTaskProcessor _receiveTaskProcessor;
        
        MyAckProcessor _ackProcessor;
        
        bool _channelFinalized;
        
    public:
        void CreateHelloQueue(AMQP::ExchangeType exchangeType, MyAMQPRoutingInfo const& routingInfo);
        
        void SendHelloWorld(std::string const& exchange, std::string const& key, std::string const& greeting);
        
        void SubscribeToReceive(std::string const& queue,
                                std::function<void(std::string const &, int64_t, bool)> const &handler,
                                bool threaded);
        
        // Class is final, virtual not needed.
        ~MyAMQPClientImpl();
        
        /**
         *  Method that is called when data needs to be sent over the network
         *
         *  Note that the AMQP library does no buffering by itself. This means
         *  that this method should always send out all data or do the buffering
         *  itself.
         *
         *  @param  connection      The connection that created this output
         *  @param  buffer          Data to send
         *  @param  size            Size of the buffer
         */
        void onData(AMQP::Connection *connection, const char *buffer, size_t size) override;
        
        /**
         *  When the connection ends up in an error state this method is called.
         *  This happens when data comes in that does not match the AMQP protocol
         *
         *  After this method is called, the connection no longer is in a valid
         *  state and can no longer be used.
         *
         *  This method has an empty default implementation, although you are very
         *  much advised to implement it. When an error occurs, the connection
         *  is no longer usable, so you probably want to know.
         *
         *  @param  connection      The connection that entered the error state
         *  @param  message         Error message
         */
        void onError(AMQP::Connection *connection, const char *message) override;
        
        /**
         *  Method that is called when the login attempt succeeded. After this method
         *  is called, the connection is ready to use. This is the first method
         *  that is normally called after you've constructed the connection object.
         *
         *  According to the AMQP protocol, you must wait for the connection to become
         *  ready (and this onConnected method to be called) before you can start
         *  using the Connection object. However, this AMQP library will cache all
         *  methods that you call before the connection is ready, so in reality there
         *  is no real reason to wait for this method to be called before you send
         *  the first instructions.
         *
         *  @param  connection      The connection that can now be used
         */
        void onConnected(AMQP::Connection *connection) override;
        
        /**
         *  Method that is called when the connection was closed.
         *
         *  This is the counter part of a call to Connection::close() and it confirms
         *  that the connection was correctly closed.
         *
         *  @param  connection      The connection that was closed and that is now unusable
         */
        void onClosed(AMQP::Connection *connection) override;
        
        void Open(MyLoginCredentials const& loginInfo);
        
        void Close();
        
    private:
        MyAMQPClientImpl(std::unique_ptr<MyNetworkConnection> networkConnection);

        size_t OnNetworkRead(char const* buf, int len);
        
        void OnNetworkReadError(std::string const& errorStr);
        
        void AckMessage(int64_t deliveryTag);

        AMQP::MessageCallback CreateInlineMessageCallback(MyMessageCallback const& userHandler);
        
        AMQP::MessageCallback CreateThreadedMessageCallback(MyMessageCallback const& userHandler);
    };
}

#endif /* defined(__AMQP__MyAMQPClientImpl__) */
