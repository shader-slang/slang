// Diagnostic test: two conformances of the same interface explicitly reserved with the same
// sequential ID. The collision is ambiguous (only one arm of the dispatch could win), so
// code generation must report an error naming the conflicting conformance instead of silently
// picking one of them.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -conformance "TriHit:IHit=0" -conformance "CurveHit:IHit=0"

interface IHit
{
    int tag();
}

export struct TriHit : IHit
{
    int d;
    int tag() { return 100; }
}

export struct CurveHit : IHit
{
    int d;
    int tag() { return 200; }
}

uniform int dynId;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IHit h = createDynamicObject<IHit>(dynId, 0);
    outputBuffer[0] = h.tag();
    //CHECK: error E50105
    //CHECK: Sequential ID '0' is assigned to multiple conformances of interface
    //CHECK: previous conformance with the same sequential ID is here
}
