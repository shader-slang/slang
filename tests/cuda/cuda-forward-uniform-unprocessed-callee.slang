//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// The second decline reason in `getForwardableParamStorage`: a callee that `shouldProcessFunction`
// refuses to rewrite. `cuda-forward-uniform-signature-preserved-callee.slang` covers the
// `hasSignaturePreservingUse` reason; this one covers `!shouldProcessFunction(callee)`, which is a
// distinct path - `exported_read` below is only ever *called*, so it has no signature-preserving
// use at all, yet its parameters must still stay by value.
//
// `[CudaDeviceExport]` gives the helper an externally visible signature that the pass deliberately
// leaves alone (`shouldProcessFunction` returns false on `CudaDeviceExportDecoration`). Forwarding
// `&big` into it would hand a `Big_0 *` to a by-value `Big_0` parameter, which NVRTC rejects.
//
// Two callees are needed to reach the decline at all. `getForwardableParamStorage` is only
// consulted from `updateCallSites`, which runs for a function the pass *did* rewrite: `normal_read`
// is that function and makes the forward be attempted for `big`, while `exported_read` is the one
// the gate then has to decline. With only the exported callee present, no rewrite happens anywhere,
// the gate is never reached, and a single-callee test would pass even with the check removed.
//
// Eligibility is all-or-nothing across the parameter, so the decline costs `normal_read` its
// forward too: `big` reaches *both* callees the ordinary way. That is the conservative direction -
// a missed optimization for one callee instead of an ABI mismatch for the other.

struct Big
{
    float v[64];
}

[noinline]
float normal_read(Big b, uint i)
{
    return b.v[i];
}

[CudaDeviceExport]
float exported_read(Big b, uint i)
{
    return b.v[i];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform Big big,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = normal_read(big, tid.x) + exported_read(big, tid.x);
}

// `normal_read`'s slot is still rewritten to a pointer - that rewrite is what brings the gate into
// play - but note below that it does not receive `&big` itself.
// CHECK: {{.*}}float normal_read_0(Big_0 * {{[A-Za-z0-9_]+}}, uint

// `exported_read` keeps its externally visible by-value signature.
// CHECK: {{.*}}float exported_read(Big_0 {{[A-Za-z0-9_]+}}, uint

// CHECK: __global__ void computeMain(Big_0 [[BIG:[A-Za-z0-9_]+]], RWStructuredBuffer<float>
// Eligibility is all-or-nothing across the parameter: because one call use (`exported_read`) is
// declined, `big` is not forwarded to *any* callee, so even the rewritten `normal_read_0` reaches
// it through the ordinary per-thread temp copy. Pinning the copy is what discriminates - with the
// callee check removed, this becomes `normal_read_0(&big_0,` and no spill is emitted.
// CHECK: Big_0 [[TMP:[A-Za-z0-9_]+]] = [[BIG]];
// CHECK: normal_read_0(&[[TMP]],
// And the declined callee must receive a value. `exported_read(&[[BIG]],` here would be the
// miscompile: a `Big_0 *` argument reaching a by-value `Big_0` parameter.
// CHECK: exported_read([[BIG]],
