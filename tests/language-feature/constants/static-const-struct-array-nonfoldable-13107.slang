// Companion regression test for shader-slang/slang#13107.
//
// The fix makes `isSimpleConstantType` accept POD structs so a `static const`
// struct table can stay a global constant, and folds synthesized member-wise
// constructor calls to `makeStruct`. Not every module-scope constructor call is
// foldable, though: a derived struct's constructor initializes its base through
// a nested base-constructor call, and a user-defined constructor may run
// arbitrary logic. `legalizeConstantConstructorCallsForGlobalScope` leaves those
// calls alone.
//
// Such an unfolded, struct-typed call must NOT be treated as a legal global
// constant just because its result type is now a "simple constant" struct — a
// call is a runtime computation. If it were left at module scope it would emit
// an illegal dynamic global initializer (a `__device__` / `constant` variable
// initialized by a constructor call, which NVRTC rejects). It must instead be
// inlined into its use sites, exactly as before the type gate was widened. This
// test guards that regression: the tables below are reconstructed locally, not
// emitted as constructor-initialized device globals.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none

struct Base
{
    uint x;
}

struct Derived : Base
{
    uint y;
}

static const Derived derivedTable[2] = {{{1}, 2}, {{3}, 4}};

struct WithCtor
{
    uint a;
    uint b;
    __init(uint v)
    {
        a = v;
        b = v * 2u;
    }
}

static const WithCtor userCtorTable[2] = {WithCtor(5u), WithCtor(6u)};

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    outBuf[0] = derivedTable[i].x + derivedTable[i].y + userCtorTable[i].a + userCtorTable[i].b;
}

// Neither table may be emitted as a constructor-initialized device global (the
// pre-fix regression). These `-NOT` lines come before the `computeMain` anchor
// so FileCheck scans the module-scope region — emitted before the entry point —
// where such a `__device__` global would appear.
// CUDA-NOT: __device__{{.*}}derivedTable
// CUDA-NOT: __device__{{.*}}userCtorTable
// The tables are instead reconstructed locally inside the entry point.
// CUDA: computeMain
// CUDA: derivedTable
// CUDA: userCtorTable
