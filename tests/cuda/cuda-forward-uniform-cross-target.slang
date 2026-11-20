//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute -line-directive-mode none

// `transformByValueParamsToBorrows` also runs for CPU, but only CUDA targets retype entry-point
// uniform parameters. On CPU the uniform aggregate is already gathered into an `EntryPointParams`
// block reached through the `entryPointParams` pointer, so the helper receives an address without
// the retype. The checks pin that ordinary CPU codegen.

struct Big
{
    float v[256];
}

[noinline]
float readsum(Big b, uint i)
{
    return b.v[i] + b.v[i + 1];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uniform Big big, uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = readsum(big, tid.x);
}

// Uniforms are collected behind the entry-point param block, and the helper reads the aggregate
// through that pointer - the ordinary CPU path. `big` is never a parameter of the emitted function
// at all (it lives in the collected block), so there is no by-value kernel parameter here for the
// forward to retype even if the CUDA gate were bypassed. That is what makes the CPU side a
// structural no-op rather than a gated one.
// CHECK: struct EntryPointParams_0
// CHECK: void _computeMain(void* {{[A-Za-z0-9_]+}}, void* {{[A-Za-z0-9_]+}}, void* {{[A-Za-z0-9_]+}})
// CHECK: readsum_0(&{{.*}}EntryPointParams_0{{.*}}big
