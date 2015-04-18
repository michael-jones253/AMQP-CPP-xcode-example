/*
 *  MyAMQP.h
 *  MyAMQP
 *
 *  Created by Michael Jones on 17/04/2015.
 *  Copyright (c) 2015 Michael Jones. All rights reserved.
 *
 */

#include "MyAMQP.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>


using namespace std;

void MyAMQP::HelloWorld(const char *greeting) {
    cout << greeting << endl;    
}

MyAMQP::MyAMQP(std::unique_ptr<MyAMQPNetworkConnection> networkConnection) {
    _networkConnection = move(networkConnection);
    
}


void MyAMQP::onData(AMQP::Connection *connection, const char *buffer, size_t size) {
    // report what is going on
    cout << "onData: " << size << endl;
    
    for (unsigned i=0; i<size; i++) cout << (int)buffer[i] << " ";
    cout << endl;
    
    
    // send to the socket
    auto bytesWritten = send(_socketFd, buffer, size, 0);
    
    cout << " FIX ME loop needed, written: " << bytesWritten << endl;
}

void MyAMQP::onError(AMQP::Connection *connection, const char *message) {
}

void MyAMQP::onConnected(AMQP::Connection *connection) {
    // show
    cout << "AMQP login success" << endl;

    // create channel if it does not yet exist
    // if (!_channel) _channel = make_shared<AMQP::Channel>(connection);
}

void MyAMQP::onClosed(AMQP::Connection *connection) {
}

void MyAMQP::Connect() {
    cout << "connecting" << endl;
    
    Close();
    
    // Setup socket and connect it.
    _socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketFd < 0)
    {
        string err = "sock you";
        throw runtime_error(err);
    }
    
    struct sockaddr_in sin_you;
    bzero(&sin_you, sizeof(sin_you));

    sin_you.sin_family = AF_INET;
    sin_you.sin_port = htons(5672);
    string yourInet = "127.0.0.1";
    auto ret = inet_aton(yourInet.c_str(), &sin_you.sin_addr);
    
    if (ret < 0)
    {
        string err = "you aton";
        throw runtime_error(err);
    }
    
    // Connect to you on send socket.
    int con_ret = connect(_socketFd, (sockaddr*)&sin_you, sizeof(sin_you));
    if (con_ret < 0)
    {
        perror(nullptr);
        string err = "connect you";
        throw runtime_error(err);
    }

    
    // create amqp connection, and a new channel
    _amqpConnection = unique_ptr<AMQP::Connection>(new AMQP::Connection(this, AMQP::Login("guest", "guest"), "/"));
    _amqpConnection->login();
}

void MyAMQP::MainLoop() {
    while (true) {
        char buf[1024];
        auto ret = recv(_socketFd, buf, sizeof(buf), 0);
        if (ret < 0) {
            break;
        }
        
        auto parsedBytes = _amqpConnection->parse(buf, ret);
        if (parsedBytes < ret) {
            // FIX ME - need to buffer.
        }
    }
}

void MyAMQP::Close() {
    
}
