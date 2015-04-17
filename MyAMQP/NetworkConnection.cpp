//
//  NetworkConnection.cpp
//  AMQP
//
//  Created by Michael Jones on 18/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "NetworkConnection.h"

#include <stdio.h>

NetworkConnection::NetworkConnection(std::function<int(char* buf, int len)> onBytes, std::function<void(std::string const &errString)> onError) {
    
}

void NetworkConnection::Open() {
    
}

void NetworkConnection::Close() {
    
}
