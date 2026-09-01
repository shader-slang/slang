//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry rayGenBool -stage raygeneration

// Regression test for the storage-legalized variant of shader-slang/slang#10188: a `bool`
// member makes `lowerBufferElementTypeToStorageType` insert a `BitCast`/`GetOffsetPtr` on the
// SBT access chain. `getRootAddr` does not peel those, so the superseded #11152 guard missed
// this shape and re-emitted `__ldg`; the shape-independent fix must still exclude it here.

struct SbtData
{
    bool flag;
    uint value;
};

// `CHECK-NOT` brackets the anchor so a regressed `__ldg` is caught wherever it lands.
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer
//CHECK-NOT: __ldg

[shader("raygeneration")]
void rayGenBool(RWStructuredBuffer<uint> output, uniform SbtData sbt)
{
    uint3 idx = DispatchRaysIndex();
    uint3 dim = DispatchRaysDimensions();
    uint k = idx.y * dim.x + idx.x;
    output[k] = (sbt.flag ? sbt.value : 0u) + k;
}
