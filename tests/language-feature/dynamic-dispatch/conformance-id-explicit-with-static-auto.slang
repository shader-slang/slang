// Runtime test: explicit conformance IDs (TriHit=0, CurveHit=1) alongside a conformance that
// is only statically referenced in the shader (InvalidHit) and therefore gets an implicitly
// assigned ID. The implicit ID must skip past the explicit reservations, so dispatch by
// sequential ID reaches TriHit and CurveHit, and the statically constructed InvalidHit still
// dispatches to its own tag.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT: set hitIDs = ubuffer(data=[0 1 99], stride=4)
StructuredBuffer<uint> hitIDs;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

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

struct InvalidHit : IHit
{
    int d;
    int tag() { return -1; }
};

IHit make(uint t)
{
    if (t == 99)
        return InvalidHit();
    return createDynamicObject<IHit, int>(t, 0);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = make(hitIDs[0]).tag(); // CHECK: 100
    outputBuffer[1] = make(hitIDs[1]).tag(); // CHECK: 200
    outputBuffer[2] = make(hitIDs[2]).tag(); // CHECK: -1
}
