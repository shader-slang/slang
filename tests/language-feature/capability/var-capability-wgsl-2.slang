//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target wgsl -entry Main -stage compute

//CHECK_IGNORE_CAPS-NOT: E36107
[require(cpp_glsl_hlsl_metal_spirv)]
struct inputWrapper {
    RWBuffer<float> InputTyped;
};

inputWrapper input;

[numthreads(64, 1, 1)]
void Main(uint3 DTid : SV_DispatchThreadID)
{
    // We should not see an error here, because the inputWrapper is not referenced.
}