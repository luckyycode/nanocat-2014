/*
 <samplecode>
 <abstract>
 Composition shader
 </abstract>
 </samplecode>
 */

#include <metal_graphics>
#include <metal_geometric>
#include <metal_graphics>
#include <metal_texture>
#include <metal_matrix>
#include <metal_math>

#include "common.h"

using namespace metal;
struct FragOutput {
    float4 albedo [[color(0)]];
    float4 normal [[color(1)]];
    float  depth [[color(2)]];
    float4 light [[color(3)]];
};


struct VertexOutput {
	float4 position [[position]];
};

vertex VertexOutput composition_vert(constant float2 *posData [[buffer(0)]],
                                     uint vid [[vertex_id]] )
{
	VertexOutput output;
	output.position = float4(posData[vid], 0.0f, 1.0f);
	return output;
}

// This fragment program will write its output to color[0], effectively overwriting the contents of gBuffers.albedo
fragment float4 composition_frag(VertexOutput in [[stage_in]]/*,
                                 FragOutput gBuffers*/)
{

    
    return  float4( 0.7, 0.1, 0.1, 1.0 );
}

