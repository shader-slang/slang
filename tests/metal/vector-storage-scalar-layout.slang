//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -force-glsl-scalar-layout

// Vectors in Metal device buffers use natural (scalar-aligned, tightly
// packed) layout by default and are lowered to MSL packed vectors (e.g.
// `packed_float3`: 12-byte stride, 4-byte alignment). This also applies to
// `float2`/`float4` elements whose native stride already matches natural
// layout: natural layout (and the reflection built on it) only guarantees
// the *element's* alignment for the buffer contents, which the native
// (size-aligned) MSL vector types cannot express.
//
// `-force-glsl-scalar-layout` requests the same layout explicitly, so both
// commands above must produce identical output.

struct Wrapped
{
    float scale;
    float4 v; // natural offset 4 violates MSL float4 alignment, must be lowered
};

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

// Struct fields with vectors are lowered to packed vectors, which have
// element alignment and no padding.
// METAL: struct Wrapped_natural
// METAL: float scale
// METAL: packed_float4 v

// METAL-NOT: {{[ (,]float3 device\* positions}}
// METAL-DAG: packed_float3 device* positions
// METAL-DAG: packed_float4 device* colors
// METAL-DAG: packed_float2 device* uvs
// METAL-NOT: {{[ (,]float3 device\* positions}}
