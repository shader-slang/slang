//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_POS): -target cuda -entry positiveMain -stage compute

// Module-scope `uniform`/global variables are folded into the synthesized
// `extern "C" __constant__ SLANG_globalParams` group, so loads of their inline data read from
// CUDA constant memory. `__ldg` (NVRTC `ld.global.nc`) needs a global-memory address operand, so
// it must not be used for these loads. Regression test for shader-slang/slang#13088.

uniform uint gScalar;
uniform uint gArr[4];

// A standalone `bool` and 16-bit scalars are the small types most susceptible to storage
// legalization. On the CUDA `__constant__` path they are not legalized: their loads root at the
// group via `FieldAddress`/`GetElementPtr` (no `BitCast`/`GetOffsetPtr`), so `CHECK-NOT: __ldg`
// holds for them too.
uniform bool gFlag;
uniform half gHalf;
uniform uint16_t gU16;

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
                (gBool.flag ? gBool.value : 0u) + (gFlag ? 7u : 0u) +
                (uint)gHalf + (uint)gU16;
}

// A top-level `ConstantBuffer<T>` folds into the group as a pointer field, so reading a member is a
// `kIROp_Load` whose root is the *loaded* CB pointer (a `Load`, which `getRootAddr` does not peel),
// not the group param. The guard does not fire and `__ldg` is kept (the data is genuinely global).
// Unlike the negative cases above, this exercises the guard's keep-`__ldg` branch: a `kIROp_Load`
// the guard must not over-suppress.
ConstantBuffer<Foo> gCB;

//CHECK_POS: __ldg
[shader("compute")]
[numthreads(1, 1, 1)]
void positiveMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = gCB.a + tid.x;
}
