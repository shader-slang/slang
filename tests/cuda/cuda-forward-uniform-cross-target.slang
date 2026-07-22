//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute -line-directive-mode none

// The address-forward (#11774) is gated to CUDA, but `transformParamsToConstRef` itself also runs
// for CPU. This is the non-CUDA side of that boundary: the same `uniform Big` -> `[noinline]` helper
// shape as cuda-forward-uniform-struct-forward.slang, emitted for CPU, must keep its normal
// collected-parameter codegen and must never produce the CUDA-only `PhysicalParamStorage<T>` type.
//
// On CPU the uniform aggregate is gathered into an `EntryPointParams` block and reached through the
// `entryPointParams` pointer, so the callee already receives an address (`&(...)->big`) - there is
// no per-thread by-value copy to eliminate, and the CUDA forward is a structural no-op here. A
// regression that let the forward fire (or reach a pointerless backend) would surface the pointer
// type, which the emitters `SLANG_UNEXPECTED`; the negative check below pins that it does not.

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
// through that pointer - the ordinary CPU path, unchanged by the CUDA-only forward.
// CHECK: struct EntryPointParams_0
// CHECK: readsum_0(&{{.*}}EntryPointParams_0{{.*}}big

// The CUDA-only forward type must never appear on a non-CUDA target.
// CHECK-NOT: PhysicalParamStorage
