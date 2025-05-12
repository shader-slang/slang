//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry Main -stage compute
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target wgsl -entry Main -stage compute -ignore-capabilities

//CHECK:error 36108: 'input' has dependencies that are not compatible on the required target 'wgsl'.
//CHECK_IGNORE_CAPS-NOT: error 36108
[require(cpp_cuda_glsl_hlsl_metal_spirv)]
struct inputWrapper {
    RWBuffer<float> InputTyped;
};

inputWrapper input;

[numthreads(64, 1, 1)]
void Main(uint3 DTid : SV_DispatchThreadID)
{

}