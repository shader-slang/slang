//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry testFragmentOutput
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry testDepthOutput  
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry testPositionInput

// Test that output-only semantics error when used as input

// CHECK: error 40015

// SV_Depth is output-only in fragment, should error as input
[shader("fragment")]
float4 testFragmentOutput(
    float depth : SV_Depth  // ERROR: SV_Depth is output-only
) : SV_Target
{
    return float4(depth, 0, 0, 1);
}

// SV_StencilRef is output-only in fragment, should error as input
[shader("fragment")]
float4 testDepthOutput(
    int stencil : SV_StencilRef  // ERROR: SV_StencilRef is output-only
) : SV_Target
{
    return float4(stencil, 0, 0, 1);
}

// SV_Position is input in fragment (gl_FragCoord) but output in vertex
// Test that it errors as input in vertex
[shader("vertex")]
float4 testPositionInput(
    float4 pos : SV_Position  // ERROR: SV_Position is output in vertex, not input
) : SV_Position
{
    return pos;
}

