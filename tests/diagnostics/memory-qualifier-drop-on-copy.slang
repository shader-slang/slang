//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

globallycoherent ByteAddressBuffer inA;
volatile ByteAddressBuffer inV;
globallycoherent int gScalar;
RWByteAddressBuffer outData;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: error[E30048]
    ByteAddressBuffer x = inA;

    // CHECK: error[E30048]
    ByteAddressBuffer p = (inA);

    // CHECK: error[E30048]
    var xi = inA;

    ByteAddressBuffer y;
    // CHECK: error[E30048]
    y = inA;

    ByteAddressBuffer q;
    // CHECK: error[E30048]
    (q) = inA;

    // CHECK: error[E30048]
    ByteAddressBuffer v = inV;

    // CHECK: error[E30048]
    var vi = inV;

    ByteAddressBuffer w;
    // CHECK: error[E30048]
    w = inV;

    // A qualified scalar read into a plain value is a completed coherent load, not a handle copy,
    // so it must not be diagnosed.
    // CHECK-NOT: error[E30048]
    int si = gScalar;

    outData.Store(0, x.Load(tid.x) + (uint)si);
}
