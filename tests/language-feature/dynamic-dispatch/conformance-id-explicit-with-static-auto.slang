// Regression test for https://github.com/shader-slang/slang/issues/14698 (case A).
//
// TriHit and CurveHit get explicit conformance IDs 0 and 1. InvalidHit is never registered;
// it is only constructed statically, so the compiler assigns its ID automatically. The
// automatic ID must skip the explicit ones, otherwise InvalidHit takes ID 0 and
// `createDynamicObject<IHit>(0, ...)` builds an InvalidHit instead of a TriHit.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT: set hitIDs = ubuffer(data=[0 1 99], stride=4)
StructuredBuffer<uint> hitIDs;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: type_conformance TriHit:IHit = 0
//TEST_INPUT: type_conformance CurveHit:IHit = 1

interface IHit
{
    int tag();
}

struct TriHit : IHit
{
    int d;
    int tag() { return 100; }
}

struct CurveHit : IHit
{
    int d;
    int tag() { return 200; }
}

struct InvalidHit : IHit
{
    int d;
    int tag() { return -1; }
}

IHit make(uint id)
{
    // 99 selects the statically constructed InvalidHit; it matches hitIDs[2] above.
    if (id == 99)
        return InvalidHit();
    return createDynamicObject<IHit, int>(id, 0);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 100
    outputBuffer[0] = make(hitIDs[0]).tag();
    // CHECK: 200
    outputBuffer[1] = make(hitIDs[1]).tag();
    // CHECK: -1
    outputBuffer[2] = make(hitIDs[2]).tag();
}
