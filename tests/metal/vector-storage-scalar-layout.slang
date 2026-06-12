//TEST:SIMPLE(filecheck=SCALAR): -target metal -stage compute -entry computeMain -force-glsl-scalar-layout
//TEST:SIMPLE(filecheck=DEFAULT): -target metal -stage compute -entry computeMain

// With `-force-glsl-scalar-layout`, vectors in device buffers use natural
// (scalar-aligned, tightly packed) layout and are lowered to MSL packed
// vectors (e.g. `packed_float3`: 12-byte stride, 4-byte alignment). This
// also applies to `float2`/`float4` elements whose native stride already
// matches natural layout: scalar layout (and the reflection built on it)
// only guarantees the *element's* alignment for the buffer contents, which
// the native (size-aligned) MSL vector types cannot express.
//
// Without the option, buffers keep the native MSL layout that reflection
// reports, so no lowering takes place.

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

// Struct fields with alignment-sensitive vectors are lowered to packed
// vectors, which have element alignment and no padding.
// SCALAR: struct Wrapped_natural
// SCALAR: float scale
// SCALAR: packed_float4 v

// SCALAR-NOT: {{\bfloat3 device\* positions}}
// SCALAR-DAG: packed_float3 device* positions
// SCALAR-DAG: packed_float4 device* colors
// SCALAR-DAG: packed_float2 device* uvs
// SCALAR-NOT: {{\bfloat3 device\* positions}}

// DEFAULT-NOT: packed_
// DEFAULT: struct Wrapped
// DEFAULT: float scale
// DEFAULT: float4 v
// DEFAULT-DAG: float3 device* positions
// DEFAULT-DAG: float4 device* colors
// DEFAULT-DAG: float2 device* uvs
// DEFAULT-NOT: packed_
