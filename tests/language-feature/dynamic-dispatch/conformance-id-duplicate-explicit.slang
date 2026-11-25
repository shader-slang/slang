// TriHit and CurveHit are both given explicit conformance ID 0 for IHit. Only one of them could
// be reached from ID 0, so the compiler reports the collision instead of silently dropping one.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain -conformance "TriHit:IHit=0" -conformance "CurveHit:IHit=0"

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

uniform uint hitID;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = createDynamicObject<IHit, int>(hitID, 0).tag();
    //CHECK: error E50105
    //CHECK: Sequential ID '0' is assigned to both
}
