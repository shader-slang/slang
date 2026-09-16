//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

globallycoherent ByteAddressBuffer inA;
volatile ByteAddressBuffer inV;
globallycoherent RWStructuredBuffer<int> inSB;
globallycoherent RWTexture2D<float> inTex;
readonly RWByteAddressBuffer inR;
writeonly RWByteAddressBuffer inW;
globallycoherent int gScalar;
globallycoherent SamplerState gSampler;
globallycoherent RaytracingAccelerationStructure gRas;
ByteAddressBuffer inPlain;
RWByteAddressBuffer outData;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: error[E30048]
    ByteAddressBuffer x = inA;

    // CHECK: error[E30048]
    ByteAddressBuffer p = (inA);

    // A `var` copy is diagnosed in the header phase (deriveVarTypeFromInitExpr), a distinct call
    // site from the body-phase check; pin exactly one diagnostic so a future double-fire regresses.
    // CHECK-COUNT-1: error[E30048]
    // CHECK-NOT: error[E30048]
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

    // A writeonly source on the type-inferred path: this path has no separate read-legality check,
    // so the dropped qualifier surfaces directly as E30048 (unlike the explicit-typed/assignment
    // paths below, which reject the illegal read first with E30119).
    // CHECK: error[E30048]
    var wq = inW;

    // Other resource-handle kinds reject the drop too: StructuredBuffer and texture.
    // CHECK: error[E30048]
    RWStructuredBuffer<int> sbCopy = inSB;

    // CHECK: error[E30048]
    RWTexture2D<float> texCopy = inTex;

    // A non-coherent/volatile memory qualifier (readonly) is likewise rejected on a copy to a local.
    // CHECK: error[E30048]
    RWByteAddressBuffer r = inR;

    // A qualified scalar read into a plain value is a completed coherent load, not a handle copy,
    // so it must not be diagnosed.
    // CHECK-NOT: error[E30048]
    int si = gScalar;

    // A sampler is an opaque handle but not a coherent memory resource, so dropping a qualifier on
    // a copy of it is out of scope and must not be diagnosed.
    // CHECK-NOT: error[E30048]
    SamplerState sCopy = gSampler;

    // A raytracing acceleration structure is a read-only opaque handle (a UntypedBufferResourceType
    // subclass), not coherent read/write memory, so a copy of it must not be diagnosed either.
    // CHECK-NOT: error[E30048]
    RaytracingAccelerationStructure rasCopy = gRas;

    // A type-mismatched copy draws only the type error, never a spurious qualifier-drop error: the
    // check runs after coercion and is skipped once the copy is known to be otherwise invalid.
    // CHECK: error[E30019]
    // CHECK-NOT: error[E30048]
    int bad = inA;

    int z;
    // CHECK: error[E30019]
    // CHECK-NOT: error[E30048]
    z = inA;

    // Assigning a qualified handle to a non-l-value (an immutable local) draws only the l-value
    // error, never a spurious qualifier-drop error.
    let immBuf = inPlain;
    // CHECK: error[E30011]
    // CHECK-NOT: error[E30048]
    immBuf = inA;

    // A destination whose type is itself ill-formed draws only that error, never a qualifier-drop
    // error (the destination-type guard rejects a non-handle / error-typed target).
    // CHECK: error[E30015]
    // CHECK-NOT: error[E30048]
    UndefinedTypeXYZ badDest = inA;

    // On the explicit-typed and assignment paths a writeonly source is caught first as an illegal
    // read (E30119); the validity gate then suppresses a redundant E30048.
    // CHECK: error[E30119]
    // CHECK-NOT: error[E30048]
    RWByteAddressBuffer we = inW;

    outData.Store(
        0,
        x.Load(tid.x) + (uint)si + (uint)sbCopy[0] + (uint)texCopy[tid.xy] + r.Load(0));
}
