//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry Main -stage compute
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target wgsl -entry Main -stage compute -ignore-capabilities

//CHECK:error 36108: 'InputTyped' has dependencies that are not compatible on the required target 'wgsl'.
//CHECK_IGNORE_CAPS-NOT: error 36108
struct inputWrapper {
    [require(cpp_cuda_glsl_hlsl_metal_spirv)]
    RWBuffer<float> InputTyped;
};

struct outputWrapper {
    [require(cpp_cuda_glsl_hlsl_metal_spirv)]
    RWBuffer<float> OutputTyped;
};

inputWrapper input;
outputWrapper output;

[numthreads(64, 1, 1)]
void Main(uint3 DTid : SV_DispatchThreadID)
{
    output.OutputTyped[DTid.x] = input.InputTyped[DTid.x];
}