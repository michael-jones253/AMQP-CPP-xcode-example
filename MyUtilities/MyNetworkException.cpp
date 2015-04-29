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

namespace  {
    class ErrorBuilder final : public std::string {
    public:
        ErrorBuilder(std::string message, bool useSysError) :
        std::string(message) {
            if (useSysError) {
                append(": ");
                append(MyUtilities::MyNetworkUtilities::SysError());
            }
        }
    };
}

namespace MyUtilities {
    
    MyNetworkException::MyNetworkException(string message, bool useSysError) :
        runtime_error{ErrorBuilder(message, useSysError)} {
        
    }

}