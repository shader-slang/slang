//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry rayGen -stage raygeneration
//TEST:SIMPLE(filecheck=CHECK_STRUCT): -target cuda -entry rayGenStruct -stage raygeneration

// Regression test for shader-slang/slang#10188.
//
// Raygen `uniform` parameters are loaded via `optixGetSbtDataPointer()`, which
// returns a pointer into the OptiX shader binding table. Hosts (e.g. slang-rhi)
// mutate SBT contents in place between dispatches without invalidating the
// non-coherent read-only cache, so loads from this pointer must NOT be lowered
// to `__ldg`. Regressing this would silently return stale uniform values on
// subsequent dispatches.

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

// Same guard, but for a struct-shaped uniform — the realistic SBT shape, where
// loads route through `FieldAddress` chains rooted at `GetOptiXSbtDataPtr`.

struct SbtData
{
    uint color;
    float scale;
};

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
