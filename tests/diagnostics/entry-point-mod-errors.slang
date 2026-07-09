// Test to check if we emit a warning when unhandled modifiers are applied to
// entry point parameters.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -entry computeMain -target cuda
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -entry computeMain -target metal
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -entry computeMain -target cpp
// `[[vk::binding]]` is only honored on Khronos/WGPU targets, so on D3D the
// "ignored modifier" diagnostic must still fire for it (alongside push_constant,
// register, and packoffset, which are unsupported on every target's entry point).
// We use `-target hlsl` rather than `-target dxil` so the check does not depend on
// a downstream DXC being installed.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -entry computeMain -target hlsl
[shader("compute")]
[numthreads(1,1,1)]
void computeMain(
    [[vk::binding(5, 4)]] RWStructuredBuffer<uint> test1,
//CHECK:                                           ^^^^^ modifier on entry point parameter is unsupported
//CHECK:                                           ^^^^^ attribute '[[vk::binding(...)]]' on parameter 'test1' is unsupported on entry point parameters and will be ignored
    [[vk::push_constant]] RWStructuredBuffer<float> test2,
//CHECK:                                            ^^^^^ modifier on entry point parameter is unsupported
//CHECK:                                            ^^^^^ attribute '[[vk::push_constant]]' on parameter 'test2' is unsupported on entry point parameters and will be ignored
    RWStructuredBuffer<float> test3 : register(r0),
//CHECK:                      ^^^^^ modifier on entry point parameter is unsupported
//CHECK:                      ^^^^^ keyword 'register' on parameter 'test3' is unsupported on entry point parameters and will be ignored
    float test4 : packoffset(c4),
//CHECK:  ^^^^^ modifier on entry point parameter is unsupported
//CHECK:  ^^^^^ keyword 'packoffset' on parameter 'test4' is unsupported on entry point parameters and will be ignored
    RWBuffer<float> output
)
{
    output[0] = test2[0];
}
