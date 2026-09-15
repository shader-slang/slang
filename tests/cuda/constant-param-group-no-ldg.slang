//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_POS): -target cuda -entry positiveMain -stage compute

// Regression test for shader-slang/slang#13088. Module-scope `uniform`/global variables are folded
// into the synthesized `extern "C" __constant__ SLANG_globalParams` group, so loads of their inline
// data read from CUDA *constant* memory. `__ldg` (NVRTC `ld.global.nc`) requires a *global*-memory
// address operand, so lowering such a load to `__ldg` produces illegal PTX. Immutability alone does
// not imply a global address, so the CUDA immutable-load pass must skip these loads.
//
// Root cause + candidate approach identified by @ccummingsNV in the issue.

uniform uint gScalar;
uniform uint gArr[4];

struct Foo
{
    uint a;
    float b;
}
uniform Foo gStruct;

// A struct with a `bool` member: a realistic aggregate uniform that exercises the generated
// struct-load helper path (which emitted a per-field `__ldg` wrapper before the fix).
struct WithBool
{
    bool flag;
    uint value;
}
uniform WithBool gBool;

RWStructuredBuffer<uint> outBuf;

// No load of inline `__constant__` data may be lowered to `__ldg` (nor to a `slang_ldg` helper that
// wraps `__ldg`). The `CHECK-NOT` brackets guard against a regressed `__ldg` landing anywhere in the
// emitted module (inline, or inside an earlier-emitted helper); the `SLANG_globalParams` anchor in
// the middle keeps the assertion from passing vacuously if emission ever changes.
//CHECK-NOT: __ldg
//CHECK: SLANG_globalParams
//CHECK-NOT: __ldg
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = gScalar + gArr[tid.x] + gStruct.a + (uint)gStruct.b +
                (gBool.flag ? gBool.value : 0u);
}

// Positive companion: a genuine read-only `StructuredBuffer` lives in global memory, reached through
// a pointer *loaded out of* the group, so its load roots at that `Load` (not the group param) and
// MUST still be lowered to `__ldg`. This guards against the fix over-suppressing legitimate reads.
StructuredBuffer<uint> inBuf;

//CHECK_POS: __ldg
[shader("compute")]
[numthreads(1, 1, 1)]
void positiveMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = inBuf[tid.x];
}
