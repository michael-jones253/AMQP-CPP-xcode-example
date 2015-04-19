/*
 *  MyAMQP.h
 *  MyAMQP
 *
 *  Created by Michael Jones on 17/04/2015.
 *  Copyright (c) 2015 Michael Jones. All rights reserved.
 *
 */

#ifndef MyAMQP_
#define MyAMQP_

#include "amqpcpp.h"
#include "MyAMQPNetworkConnection.h"

#include <string>
#include <mutex>
#include <condition_variable>

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    
    class MyAMQPClient : public AMQP::ConnectionHandler {
        // Open source stuff.
        std::unique_ptr<AMQP::Connection> _amqpConnection;
        
        // My stuff.
        std::unique_ptr<MyAMQPNetworkConnection> _networkConnection;
        
        // Open source stuff.
        std::unique_ptr<AMQP::Channel> _channel;
        
        std::mutex _mutex;
        
        std::condition_variable _conditional;
        
        bool _channelOpen;
        
        bool _channelInError;
        
        bool _queueReady;
        
    public:
        void HelloChannel();
        
        void HelloWorld(const char *);
        
        MyAMQPClient(std::unique_ptr<MyAMQPNetworkConnection> networkConnection);
        
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
        
        void Open(std::string const& ipAddress);
        
        void Close();
        
    private:
        size_t OnRead(char const* buf, int len);
        
        void OnReadError(std::string const& errorStr);
        
    };
}
#pragma GCC visibility pop
#endif
