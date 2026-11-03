//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target wgsl -entry Main -stage compute
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS,exit-code=-1): -target wgsl -entry Main -stage compute -ignore-capabilities

[require(cpp_cuda_glsl_hlsl_metal_spirv)]
Buffer<float> InputTyped;
[require(cpp_cuda_glsl_hlsl_metal_spirv)]
RWBuffer<float> OutputTyped;

//CHECK: error[E36107]
// With -ignore-capabilities the capability error is suppressed. The buffer accessors have no WGSL
// implementation, so their bodies end without a return, which is an error for WGSL.
//CHECK_IGNORE_CAPS-NOT: E36107
//CHECK_IGNORE_CAPS: error[E41009]: non-void function must return
//CHECK_IGNORE_CAPS-NOT: E36107
[numthreads(64, 1, 1)]
void Main(uint3 DTid : SV_DispatchThreadID)
{
    OutputTyped[DTid.x] = InputTyped[DTid.x];
}