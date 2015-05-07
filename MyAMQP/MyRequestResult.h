//
//  MyRequestResult.h
//  AMQP
//
//  Created by Michael Jones on 7/05/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#ifndef AMQP_MyRequestResult_h
#define AMQP_MyRequestResult_h

namespace MyAMQP {
    
    // For the request processor which needs to process tasks giving results of different types.
    union MyRequestResult {
        ssize_t ByteCount;
        bool    Ok;
    };
}

#endif
