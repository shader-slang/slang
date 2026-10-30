//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// `[vk::offset(N)]` on an entry-point `uniform` parameter must give a byte
// offset that is a multiple of the parameter's alignment, and the parameter
// must not share bytes with another parameter of the same entry point.

RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(
    uniform float4 a,
//CHECK:           ^ see declaration of 'a'
    [vk::offset(8)] uniform float b,
//CHECK:                          ^ overlapping entry-point uniform parameters
//CHECK:                          ^ the uniform data of 'b' overlaps the uniform data of 'a'
    [vk::offset(36)] uniform float4 c)
//CHECK: ^^^^^^ misaligned 'vk::offset'
//CHECK: ^^^^^^ '[vk::offset(36)]' is not a multiple of the parameter's alignment, 16
{
    outputBuffer[0] = a + b + c;
}
