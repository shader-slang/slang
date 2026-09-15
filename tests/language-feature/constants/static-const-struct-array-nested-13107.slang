// Companion regression test for shader-slang/slang#13107: nested POD structs.
//
// `isSimpleConstantType` recurses through struct fields, so a struct whose
// fields are themselves simple constants (here an inner struct plus a scalar) is
// a simple constant too, and `legalizeConstantConstructorCallsForGlobalScope`
// folds the nested synthesized constructor calls to nested `makeStruct`s. The
// whole table must therefore stay a global constant, with no per-invocation
// reconstruction from `$init` calls at any nesting level.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none

struct Inner
{
    uint p;
    uint q;
}

struct Nested
{
    Inner inner;
    uint tag;
}

static const Nested nestedTable[2] = {{{1, 2}, 3}, {{4, 5}, 6}};

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    outBuf[0] = nestedTable[i].inner.p + nestedTable[i].inner.q + nestedTable[i].tag;
}

// CUDA: __device__ static const FixedArray<Nested{{.*}}, 2>{{.*}}nestedTable
// CUDA-NOT: Nested{{.*}}init
// CUDA-NOT: Inner{{.*}}init
