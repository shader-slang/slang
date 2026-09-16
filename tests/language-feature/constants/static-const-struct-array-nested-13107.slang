// Companion regression test for shader-slang/slang#13107: nested POD structs.
//
// `isSimpleConstantType` recurses through struct fields, so a struct whose
// fields are themselves simple constants (here an inner struct plus a scalar) is
// a simple constant too, and `legalizeConstantConstructorCallsForGlobalScope`
// folds the nested synthesized constructor calls to nested `makeStruct`s. The
// whole table must therefore stay a global constant, with no per-invocation
// reconstruction from `$init` calls at any nesting level.
//
// The CPU value lane pins the field-key / declaration order of the recursive
// reconstruction: each lane's encoding (`inner.p*100 + inner.q*10 + tag`) is
// order-sensitive, so scrambling any of the three fields while rebuilding the
// `makeStruct`s would produce a different value and fail the buffer check. The
// table is indexed by `tid.x` (a runtime value) so it stays a real global rather
// than being constant-folded away.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

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

static const Nested nestedTable[4] = {{{1, 2}, 3}, {{4, 5}, 6}, {{7, 8}, 9}, {{10, 11}, 12}};

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    outputBuffer[i] = nestedTable[i].inner.p * 100 + nestedTable[i].inner.q * 10 + nestedTable[i].tag;
}

// CUDA: __device__ static const FixedArray<Nested{{.*}}, 4>{{.*}}nestedTable
// CUDA-NOT: Nested{{.*}}init
// CUDA-NOT: Inner{{.*}}init

// CHECK: 123
// CHECK: 456
// CHECK: 789
// CHECK: 1122
