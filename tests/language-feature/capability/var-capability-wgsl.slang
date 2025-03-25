//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry Main -stage compute
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target wgsl -entry Main -stage compute -ignore-capabilities

//CHECK:error 36108: 'InputTyped' has dependencies that are not compatible on the required target 'wgsl'.
//CHECK_IGNORE_CAPS-NOT: error 36108
[require(cpp_cuda_glsl_hlsl_metal_spirv)]
Buffer<float> InputTyped;
[require(cpp_cuda_glsl_hlsl_metal_spirv)]
RWBuffer<float> OutputTyped;

[numthreads(64, 1, 1)]
void Main(uint3 DTid : SV_DispatchThreadID)
{
    OutputTyped[DTid.x] = InputTyped[DTid.x];
}