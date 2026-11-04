//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry rayGenMain -stage raygeneration -line-directive-mode none

// OptiX shaders compile through the CUDA path, but a raygen entry point's uniforms are collected
// into the shader-record struct before `transformByValueParamsToBorrows` runs, so there is no
// by-value entry-point parameter to retype. The helper receives the shader-record address through
// the existing immutable-address path.

struct Big
{
    float v[64];
}

[noinline]
float readsum(Big b, uint i)
{
    return b.v[i] + b.v[(i + 1) % 64];
}

[shader("raygeneration")]
void rayGenMain(uniform Big big, uniform RWStructuredBuffer<float> output)
{
    output[0] = readsum(big, 3);
}

// The kernel takes no by-value parameters at all - OptiX passes them via the SBT record.
// CHECK: __global__ void __raygen__rayGenMain()

// The aggregate is reached through the shader-record pointer, not forwarded from a by-value param.
// CHECK: readsum_0(&(({{.*}}ShaderRecordParams{{.*}}*)optixGetSbtDataPointer())->big
