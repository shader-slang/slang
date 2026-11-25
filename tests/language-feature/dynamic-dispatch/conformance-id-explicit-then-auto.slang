// Regression test for https://github.com/shader-slang/slang/issues/14698 (case B1).
//
// TriHit and CurveHit get explicit conformance IDs 0 and 1, and AutoHit is registered after
// them without an ID. AutoHit's automatic ID must skip the explicit ones and become 2, so
// every ID reaches the type it was given to.
// `conformance-id-auto-then-explicit.slang` registers the same types in the opposite order.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT: set hitIDs = ubuffer(data=[0 1 2], stride=4)
StructuredBuffer<uint> hitIDs;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: type_conformance TriHit:IHit = 0
//TEST_INPUT: type_conformance CurveHit:IHit = 1
//TEST_INPUT: type_conformance AutoHit:IHit

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

struct AutoHit : IHit
{
    int d;
    int tag() { return -1; }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 100
    outputBuffer[0] = createDynamicObject<IHit, int>(hitIDs[0], 0).tag();
    // CHECK: 200
    outputBuffer[1] = createDynamicObject<IHit, int>(hitIDs[1], 0).tag();
    // CHECK: -1
    outputBuffer[2] = createDynamicObject<IHit, int>(hitIDs[2], 0).tag();
}
