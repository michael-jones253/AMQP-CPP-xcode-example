//
//  MyAMQPNetworkConnection.cpp
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyAMQPNetworkConnection.h"

#include <stdio.h>

MyAMQPNetworkConnection::MyAMQPNetworkConnection(std::function<int(char* buf, int len)> onBytes, std::function<void(std::string const &errString)> onError) {
    
}

void MyAMQPNetworkConnection::Open() {
    
}

void MyAMQPNetworkConnection::Close() {
    
}
