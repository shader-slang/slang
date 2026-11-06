//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -conformance Foo:IFoo=0

// Packing a buffer struct that has a `uint4` member into an any-value reads each element of the
// packed vector. The element 0 must be read from the vector, and not be replaced by the whole
// packed vector.

interface IFoo
{
    uint get();
}

struct Foo : IFoo
{
    uint4 h;
    float a;
    uint get() { return h.x + h.w + asuint(a); }
}

struct Blob
{
    uint4 h;
    float4 f;
};

RWStructuredBuffer<Blob> inBuf;
RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    IFoo foo = createDynamicObject<IFoo, Blob>(0, inBuf[tid.x]);
    outBuf[tid.x] = foo.get();
}

// METAL: = {{.*}}h_0[int(0)];
