//
//  MyNetworkUtilities.cpp
//  AMQP
//
//  Created by Michael Jones on 27/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyNetworkUtilities.h"
#include <sys/errno.h>

using namespace std;

namespace MyUtilities {
 
    string MyNetworkUtilities::SysError() {
        auto sysErr = strerror(errno);
        return sysErr;        
    }
}