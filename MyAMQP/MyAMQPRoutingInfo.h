//
//  MyAMQPRoutingInfo.h
//  AMQP
//
//  Created by Michael Jones on 21/04/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef AMQP_MyAMQPRoutingInfo_h
#define AMQP_MyAMQPRoutingInfo_h

/* The classes below are exported */
#pragma GCC visibility push(default)

namespace MyAMQP {
    
    // POD - plain old data.
    struct MyAMQPRoutingInfo {
        std::string ExchangeName;
        
        std::string Key;
        
        std::string QueueName;
    };    
}

#pragma GCC visibility pop


#endif
