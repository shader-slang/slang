//TEST:SIMPLE(filecheck=SCALAR): -target metal -stage compute -entry computeMain -force-glsl-scalar-layout
//TEST:SIMPLE(filecheck=DEFAULT): -target metal -stage compute -entry computeMain

// With `-force-glsl-scalar-layout`, vectors in device buffers use natural
// (scalar-aligned, tightly packed) layout. MSL `float3` has a 16-byte stride,
// so `float3` elements are lowered to MSL packed vectors (`packed_float3`:
// 12-byte stride, 4-byte alignment). `float2`/`float4` buffer elements already
// have a stride matching natural layout, so they keep their native MSL vector
// type.
//
// Without the option, buffers keep the native MSL layout that reflection
// reports, so no lowering takes place.

struct Wrapped
{
    float scale;
    float4 v; // natural offset 4 violates MSL float4 alignment, must be lowered
}

StructuredBuffer<float3> positions;
StructuredBuffer<float4> colors;
StructuredBuffer<float2> uvs;
StructuredBuffer<Wrapped> wrapped;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3 p = positions[tid.x];
    float4 c = colors[tid.x];
    float2 uv = uvs[tid.x];
    Wrapped w = wrapped[tid.x];
    output[tid.x] = p.x + p.y + p.z + c.w + uv.y + w.scale + w.v.x;
}

// Struct fields with alignment-sensitive vectors are lowered to packed
// vectors, which have element alignment and no padding.
// SCALAR: struct Wrapped_natural
// SCALAR: float scale
// SCALAR: packed_float4 v

// SCALAR-NOT: {{\bfloat3 device\* positions}}
// SCALAR-DAG: packed_float3 device* positions
// SCALAR-DAG: float4 device* colors
// SCALAR-DAG: float2 device* uvs
// SCALAR-NOT: {{\bfloat3 device\* positions}}

// DEFAULT-NOT: packed_
// DEFAULT: struct Wrapped
// DEFAULT: float scale
// DEFAULT: float4 v
// DEFAULT-DAG: float3 device* positions
// DEFAULT-DAG: float4 device* colors
// DEFAULT-DAG: float2 device* uvs
// DEFAULT-NOT: packed_
