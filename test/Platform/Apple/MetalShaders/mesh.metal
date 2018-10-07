//
//  Shaders.metal
//  MetalTriangles
//
//  Created by Warren Moore on 8/26/14.
//  Copyright (c) 2014 Metal By Example. All rights reserved.
//

#include <metal_stdlib>
#include <metal_matrix>
#include <metal_graphics>
#include <metal_texture>
#include <metal_math>

#include "common.h"

using namespace metal;

struct FragOutput {
    float4 albedo [[color(0)]];
    float4 normal [[color(1)]];
    float  depth [[color(2)]];
    float4 light [[color(3)]];
};


struct Vertex {
    float3 position [[ attribute(0) ]];
    float3 normal [[ attribute(1) ]];
    float2 texcoord [[ attribute(2) ]];
};

struct ProjectedVertex {
    float4 position [[position]];
    float3 eye;
    float3 normal;
    float2 uv;
    
    float v_linear_distepth;
};

vertex ProjectedVertex vertex_main(Vertex vert [[stage_in]],
                                   constant Uniforms &uniforms [[buffer(3)]])
{
    ProjectedVertex outVert;
    outVert.position = uniforms.modelViewProjectionMatrix * float4(vert.position, 1.0);
    outVert.eye =  -(uniforms.modelViewMatrix * float4(vert.position,1.0)).xyz;
    outVert.normal = uniforms.normalMatrix * vert.normal;
    outVert.uv = vert.texcoord;
    
 //   outVert.uv.x = 1.0f - vert.texcoord.x;
 //   outVert.uv.y = 1.0f - vert.texcoord.y;
    
    outVert.v_linear_distepth = (uniforms.modelViewMatrix * float4(vert.position, 1.0)).z;
    
    return outVert;
}

fragment FragOutput fragment_main(ProjectedVertex vert [[stage_in]],
                              constant Uniforms &uniforms [[buffer(0)]],
                                sampler           sampler2D [[ sampler(0) ]],
                              texture2d<float>  tex2D     [[ texture(0) ]]
                             
                            )
{
    //constexpr sampler linear_sampler(min_filter::linear, mag_filter::linear);
    float4 color = tex2D.sample( sampler2D, vert.uv );
    
    FragOutput output;
    
    //world_normal = world_normal * scale + 0.5;
    
    output.albedo.rgb = color.rgb;
    output.albedo.a = color.a;
    output.normal.rgb = color.rgb;
    output.normal.w = 1;
    output.depth = vert.v_linear_distepth;
    output.light = 1;
    
    return output;
}
