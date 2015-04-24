//
//  MyAMQPTypes.h
//  AMQP
//
//  Created by Michael Jones on 24/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef AMQP_MyAMQPTypes_h
#define AMQP_MyAMQPTypes_h

#include <functional>
#include <string>

namespace MyAMQP {
    using MyMessageCallback = std::function<void(std::string const& message, int64_t tag, bool redelivered)>;    
}
#endif
