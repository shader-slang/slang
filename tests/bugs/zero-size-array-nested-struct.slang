// Test that zero-size arrays inside nested structs don't cause ICE during type legalization.
// Regression test for https://github.com/shader-slang/slang/issues/10069

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute

struct Nested1 { struct Inner { float x; }; Inner data[0]; };
struct Nested2 { Nested1 a; Nested1 b; };
RWStructuredBuffer<Nested2> buf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Nested2 v = buf[tid.x];
    buf[tid.x] = v;
}

// SPIRV: OpEntryPoint
// HLSL: void computeMain
// GLSL: void main
