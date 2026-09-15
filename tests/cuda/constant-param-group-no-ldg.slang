//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_POS): -target cuda -entry positiveMain -stage compute

// Module-scope `uniform`/global variables are folded into the synthesized
// `extern "C" __constant__ SLANG_globalParams` group, so loads of their inline data read from
// CUDA constant memory. `__ldg` (NVRTC `ld.global.nc`) needs a global-memory address operand, so
// it must not be used for these loads. Regression test for shader-slang/slang#13088.

uniform uint gScalar;
uniform uint gArr[4];

// A standalone `bool` uniform is the empirical pin for this fix's design. If the CUDA
// `__constant__` path legalized `bool` storage (inserting a `BitCast`/`GetOffsetPtr`, as the
// SPIR-V/SBT path does), this load would evade a `getRootAddr`-rooted guard and re-emit `__ldg`.
// It does not — so reusing `getRootAddr` suffices and no shape-independent peel-walker is needed.
uniform bool gFlag;

struct Foo
{
    uint a;
    float b;
}
uniform Foo gStruct;

// A struct with a `bool` member exercises the aggregate struct-load path alongside the scalar and
// array paths above.
struct WithBool
{
    bool flag;
    uint value;
}
uniform WithBool gBool;

RWStructuredBuffer<uint> outBuf;

// No load of inline `__constant__` data may use `__ldg` (nor a `slang_ldg` helper that wraps it).
// The `CHECK-NOT` pair brackets a `SLANG_globalParams` anchor so the assertion cannot pass
// vacuously and catches a stray `__ldg` on either side of it.
//CHECK-NOT: __ldg
//CHECK: SLANG_globalParams
//CHECK-NOT: __ldg
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = gScalar + gArr[tid.x] + gStruct.a + (uint)gStruct.b +
                (gBool.flag ? gBool.value : 0u) + (gFlag ? 7u : 0u);
}

// A read-only `StructuredBuffer` lives in global memory, reached through a pointer loaded out of
// the group, so its load roots at that `Load` (not the group param) and must still use `__ldg`.
StructuredBuffer<uint> inBuf;

//CHECK_POS: __ldg
[shader("compute")]
[numthreads(1, 1, 1)]
void positiveMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = inBuf[tid.x];
}
