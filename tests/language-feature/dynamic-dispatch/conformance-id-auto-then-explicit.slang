// Regression test for https://github.com/shader-slang/slang/issues/14698 (case B2).
//
// AutoHit is registered first without an ID, then CurveHit, TriHit and FarHit with explicit
// IDs 1, 0 and 10. Because AutoHit comes first, a single pass over the witness tables would
// number it 0 before seeing TriHit's explicit 0. Its automatic ID must still skip the explicit
// ones and become 2.
//
// An ID missing from the dispatch mapping falls back to the type with the largest ID. FarHit
// holds that largest ID, so if AutoHit were numbered anything other than 2, input 2 would reach
// FarHit and the third CHECK would fail.
// `conformance-id-explicit-then-auto.slang` uses the opposite order.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT: set hitIDs = ubuffer(data=[0 1 2 10], stride=4)
StructuredBuffer<uint> hitIDs;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: type_conformance AutoHit:IHit
//TEST_INPUT: type_conformance CurveHit:IHit = 1
//TEST_INPUT: type_conformance TriHit:IHit = 0
//TEST_INPUT: type_conformance FarHit:IHit = 10

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

struct FarHit : IHit
{
    int d;
    int tag() { return 300; }
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
    // CHECK: 300
    outputBuffer[3] = createDynamicObject<IHit, int>(hitIDs[3], 0).tag();
}
