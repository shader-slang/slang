//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry rayGen -stage raygeneration
//TEST:SIMPLE(filecheck=CHECK_STRUCT): -target cuda -entry rayGenStruct -stage raygeneration

// Regression test for shader-slang/slang#10188: raygen `uniform`s live in the OptiX SBT,
// which the host mutates between dispatches, so their loads must NOT be lowered to `__ldg`
// (CUDA's read-only cache) or they return stale values on later dispatches.

// `CHECK-NOT` on both sides: a regressed `__ldg` can land before the anchor (in a
// `slang_ldg` helper emitted earlier) or after it (inline), so one side would miss a shape.
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer
//CHECK-NOT: __ldg

[shader("raygeneration")]
void rayGen(RWStructuredBuffer<uint> output, uniform uint value)
{
    uint3 idx = DispatchRaysIndex();
    uint3 dim = DispatchRaysDimensions();
    uint k = idx.y * dim.x + idx.x;
    output[k] = value + k;
}

// Struct-shaped uniform — the realistic SBT shape (loads via `FieldAddress` chains).

struct SbtData
{
    uint color;
    float scale;
};

//CHECK_STRUCT-NOT: __ldg
//CHECK_STRUCT: optixGetSbtDataPointer
//CHECK_STRUCT-NOT: __ldg

[shader("raygeneration")]
void rayGenStruct(RWStructuredBuffer<float> output, uniform SbtData sbt)
{
    uint3 idx = DispatchRaysIndex();
    uint3 dim = DispatchRaysDimensions();
    uint k = idx.y * dim.x + idx.x;
    output[k] = float(sbt.color) * sbt.scale + float(k);
}
