//
//  MyKeyboardInput.cpp
//  AMQP
//
//  Created by Michael Jones on 1/05/2015.
//  Copyright (c) 2015 Michael Jones. All rights reserved.
//

#include "MyKeyboardInput.h"
#include <iostream>
#include <assert.h>

using namespace std;

namespace MyUtilities {
    char MyKeyboardInput::GetOption(vector<string> optionStrings) {
        
        for (auto  const & option : optionStrings) {
            cout << option[0] << " (" << option << ")" << endl;
        }

        string userInput{};
        bool validInput{};
        while (!validInput) {
            
            getline(cin, userInput);
            
            if (userInput.length() == 0) {
                continue;
            }
            
            for (auto  const & option : optionStrings) {
                auto comparison = option.compare(0, userInput.length(), userInput);
                if (comparison == 0) {
                    validInput = true;
                    break;
                }
            }
        }
        
        assert(userInput.length() > 0);
        return userInput[0];
    }
    
}

