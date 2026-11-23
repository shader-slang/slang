// Runtime test: mixing explicit conformance IDs (`-conformance Type:Interface=ID`) with an
// implicitly assigned ID. The explicit IDs (TriHit=0, CurveHit=1) must keep their reserved
// values, and the implicit ID (AutoHit) must be allocated from the remaining free slots
// instead of colliding with the explicit ones. Dispatch by sequential ID must reach the
// right type for every registered conformance.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT: set hitIDs = ubuffer(data=[0 1 2], stride=4)
StructuredBuffer<uint> hitIDs;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: type_conformance AutoHit:IHit
//TEST_INPUT: type_conformance CurveHit:IHit = 1
//TEST_INPUT: type_conformance TriHit:IHit = 0

interface IHit
{
    int tag();
}

struct TriHit : IHit
{
    int d;
    int tag() { return 100; }
};

struct CurveHit : IHit
{
    int d;
    int tag() { return 200; }
};

struct AutoHit : IHit
{
    int d;
    int tag() { return -1; }
};

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // TriHit keeps its explicit sequential ID 0.
    outputBuffer[0] = createDynamicObject<IHit, int>(hitIDs[0], 0).tag(); // CHECK: 100
    // CurveHit keeps its explicit sequential ID 1.
    outputBuffer[1] = createDynamicObject<IHit, int>(hitIDs[1], 0).tag(); // CHECK: 200
    // AutoHit is allocated the first free ID (2), not one of the reserved slots.
    outputBuffer[2] = createDynamicObject<IHit, int>(hitIDs[2], 0).tag(); // CHECK: -1
}
