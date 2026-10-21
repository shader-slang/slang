//TEST:SIMPLE(filecheck=SINGLE): -target wgsl -stage compute -entry singleAxis
//TEST:SIMPLE(filecheck=MIXED): -target wgsl -stage compute -entry mixedTypes
//TEST:SIMPLE(filecheck=LITERAL): -target wgsl -stage compute -entry literalOnly
//TEST:SIMPLE(filecheck=UNSIGNED): -target wgsl -stage compute -entry unsignedAxis

// A specialization constant in `[numthreads]` is emitted as the name of its WGSL `override`
// in `@workgroup_size`, so the pipeline can pick the workgroup size at creation time.
// See shader-slang/slang#13767.

// WGSL requires every `@workgroup_size` argument to have the same type (i32 or u32). Literal
// axes stay as bare abstract-int literals, which convert to either type, and every override
// that is not `i32` is converted to `i32`, even when it is the only override.

// SINGLE: @id(0) override kGroupSize_0 : i32
// SINGLE: @workgroup_size(kGroupSize_0, 1, 1)

// MIXED-DAG: @id(1) override kX_0 : u32
// MIXED-DAG: @id(2) override kY_0 : i32
// MIXED: @workgroup_size(i32(kX_0), kY_0, 1)
// MIXED: writeSize_0(vec3<i32>(i32(kX_0), kY_0, i32(1)))

// LITERAL: @workgroup_size(8, 4, 2)

// UNSIGNED: @workgroup_size(i32(kX_0), 1, 1)

[vk::constant_id(0)]
const int kGroupSize = 64;

[vk::constant_id(1)]
const uint kX = 8;

[vk::constant_id(2)]
const int kY = 4;

RWStructuredBuffer<int> output;

[noinline]
void writeSize(int3 size)
{
    output[0] = size.x;
    output[1] = size.y;
    output[2] = size.z;
}

[numthreads(kGroupSize, 1, 1)]
void singleAxis(uint3 id : SV_DispatchThreadID)
{
    output[id.x] = int(id.x);
}

[numthreads(kX, kY, 1)]
void mixedTypes()
{
    writeSize(WorkgroupSize());
}

[numthreads(kX, 1, 1)]
void unsignedAxis(uint3 id : SV_DispatchThreadID)
{
    output[id.x] = int(id.x);
}

[numthreads(8, 4, 2)]
void literalOnly(uint3 id : SV_DispatchThreadID)
{
    output[id.x] = int(id.x);
}
