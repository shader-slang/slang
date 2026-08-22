//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry rayGenMain -stage raygeneration -line-directive-mode none

// The CUDA address-forward (#11774) is enabled for the whole CUDA family, which includes the PTX
// path OptiX shaders compile through - but it only ever fires on a by-value entry-point uniform
// aggregate. This pins the OptiX side of that boundary: a raygen entry point's uniforms are
// collected into the SBT shader-record struct before `transformParamsToConstRef` runs, so the
// aggregate is already reached through a pointer and there is no per-thread by-value copy to
// eliminate. `isEntryPointByValueUniformAggregateParam` therefore declines it, and the helper
// receives the shader-record address through the ordinary immutable-address path instead.

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
