//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target wgsl -entry Main -stage compute

//CHECK_IGNORE_CAPS-NOT: error 36107
[require(cpp_cuda_glsl_hlsl_metal_spirv)]
struct inputWrapper {
    RWBuffer<float> InputTyped;
};

inputWrapper input;

[numthreads(64, 1, 1)]
void Main(uint3 DTid : SV_DispatchThreadID)
{
    // We should not see an error here, because the inputWrapper is not referenced.
}