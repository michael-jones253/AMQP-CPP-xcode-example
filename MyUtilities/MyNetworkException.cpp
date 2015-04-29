//
//  MyNetworkException.cpp
//  AMQP
//
//  Created by Michael Jones on 29/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyNetworkException.h"
#include "MyNetworkUtilities.h"

using namespace std;

namespace MyUtilities {
    
    MyNetworkException::MyNetworkException(string message) :
        runtime_error{message + ": " + MyNetworkUtilities::SysError()} {
        
    }

}