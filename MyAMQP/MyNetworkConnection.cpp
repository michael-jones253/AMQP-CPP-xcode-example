//
//  MyNetworkConnection.cpp
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyNetworkConnection.h"

using namespace std;

MyNetworkConnection::MyNetworkConnection(function<int(char* buf, int len)> onBytes, function<void(string const& errString)> onError) : NetworkConnection(onBytes, onError) {
    
}

void MyNetworkConnection::Connect(string const& ipAddress, int port) {
    
}

void MyNetworkConnection::Disconnect() {
    
}

int MyNetworkConnection::Read(char* buf, int len) {
    return 0;
}

