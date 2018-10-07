//
//  common.h
//  Neko
//
//  Created by Kawaii Neko on 6/12/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef common_h
#define common_h

#include <simd/simd.h>
using namespace simd;

#ifdef __cplusplus

typedef struct
{
    float4x4 modelViewProjectionMatrix;
    float4x4 modelViewMatrix;
    float3x3 normalMatrix;
} Uniforms;

#endif
#endif /* common_h */
