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

MyAMQP::MyAMQP(std::unique_ptr<MyAMQPNetworkConnection> networkConnection) :
    _amqpConnection{},
    _networkConnection{},
    _channel{} {
    _networkConnection = move(networkConnection);
    
}


void MyAMQP::onData(AMQP::Connection *connection, const char *buffer, size_t size) {
    cout << "onData: " << size << endl;
    
    for (unsigned i=0; i<size; i++) cout << (int)buffer[i] << " ";
    cout << endl;
    
    auto bytesWritten = _networkConnection->SendToServer(buffer, size);
    
    cout << " FIX ME loop needed, written: " << bytesWritten << endl;
}

void MyAMQP::onError(AMQP::Connection *connection, const char *message) {
    cout << "AMQP error:" << message << endl;
    _networkConnection->Close();
}

void MyAMQP::onConnected(AMQP::Connection *connection) {
    cout << "AMQP login success" << endl;

    // create channel if it does not yet exist
    if (!_channel) {
        _channel = unique_ptr<AMQP::Channel>(new AMQP::Channel(connection));
    }
}

void MyAMQP::onClosed(AMQP::Connection *connection) {
}

void MyAMQP::Open(const string& ipAddress) {
    _networkConnection->Open(ipAddress,
                             bind(&MyAMQP::OnRead, this, placeholders::_1, placeholders::_2),
                             bind(&MyAMQP::OnReadError, this, placeholders::_1));
    
    _amqpConnection = unique_ptr<AMQP::Connection>(new AMQP::Connection(this, AMQP::Login("guest", "guest"), "/"));
    _amqpConnection->login();

}

void MyAMQP::Close() {
    
}

size_t MyAMQP::OnRead(char const* buf, int len) {
    auto parsedBytes = _amqpConnection->parse(buf, len);
    
    return parsedBytes;
}

void MyAMQP::OnReadError(std::string const& errorStr){
    cout << "MyAMQP read error: " << errorStr << endl;
    _networkConnection->Close();
}
