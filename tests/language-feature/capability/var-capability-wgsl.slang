//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target wgsl -entry Main -stage compute
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS,exit-code=-1): -target wgsl -entry Main -stage compute -ignore-capabilities

[require(cpp_cuda_glsl_hlsl_metal_spirv)]
Buffer<float> InputTyped;
[require(cpp_cuda_glsl_hlsl_metal_spirv)]
RWBuffer<float> OutputTyped;

//CHECK: error[E36107]
// With -ignore-capabilities the capability error is suppressed. The `__target_switch` of each
// buffer accessor then has no WGSL case, which is an error.
//CHECK_IGNORE_CAPS-NOT: E36107
//CHECK_IGNORE_CAPS: error[E41037]: __target_switch has no case for the current target
//CHECK_IGNORE_CAPS-NOT: E36107
[numthreads(64, 1, 1)]
void Main(uint3 DTid : SV_DispatchThreadID)
{
    OutputTyped[DTid.x] = InputTyped[DTid.x];
}