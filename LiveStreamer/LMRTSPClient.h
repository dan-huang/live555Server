//
//  LMRTSPClient.hpp
//  Live555Demo
//
//  Created by dan on 2018/7/19.
//  Copyright © 2018 dan. All rights reserved.
//

#ifndef LMRTSPClient_h
#define LMRTSPClient_h

#include <stdio.h>

typedef void BufferOutputFunc(u_int8_t* buffer, unsigned size);

void playRTSP(char const* rtspURL, BufferOutputFunc *func);
void stopRTSP();

#endif /* LMRTSPClient_hpp */
