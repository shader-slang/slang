//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry Main -stage compute
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target wgsl -entry Main -stage compute -ignore-capabilities

[require(cpp_cuda_glsl_hlsl_metal_spirv)]
Buffer<float> InputTyped;
[require(cpp_cuda_glsl_hlsl_metal_spirv)]
RWBuffer<float> OutputTyped;

//CHECK: ([[# @LINE+3]]): error 36107
//CHECK_IGNORE_CAPS-NOT: error 36107
[numthreads(64, 1, 1)]
void Main(uint3 DTid : SV_DispatchThreadID)
{
    OutputTyped[DTid.x] = InputTyped[DTid.x];
}