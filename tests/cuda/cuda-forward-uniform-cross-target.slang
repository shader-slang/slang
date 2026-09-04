//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute -line-directive-mode none

// The address-forward (#11774) is gated to CUDA, but `transformParamsToConstRef` itself also runs
// for CPU. This is the non-CUDA side of that boundary: the same `uniform Big` -> `[noinline]` helper
// shape as cuda-forward-uniform-struct-forward.slang, emitted for CPU, must keep its normal
// collected-parameter codegen and must never fire the CUDA-only `ConstRef<T, CudaKernelParam>`
// forward.
//
// On CPU the uniform aggregate is gathered into an `EntryPointParams` block and reached through the
// `entryPointParams` pointer, so the callee already receives an address (`&(...)->big`) - there is
// no per-thread by-value copy to eliminate, and the CUDA forward is a structural no-op here. The
// positive checks below are the discriminator: they pin that `big` is reached through the collected
// `EntryPointParams` block, not handed to the helper as a forwarded by-value kernel parameter.

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
