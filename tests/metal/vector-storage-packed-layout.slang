//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -force-glsl-scalar-layout

// Vectors in Metal device buffers are lowered to MSL packed vectors (e.g.
// `packed_float3`) so the storage matches the natural (scalar-aligned)
// layout reflection reports. This applies to `float2`/`float4` too, whose
// native size-alignment a buffer bound at element alignment cannot satisfy.
// `-force-glsl-scalar-layout` requests the same layout, so both commands
// above must produce identical output.

struct Wrapped
{
    float scale;
    float4 v;
};

StructuredBuffer<float3> positions;
StructuredBuffer<float4> colors;
StructuredBuffer<float2> uvs;
StructuredBuffer<Wrapped> wrapped;
// `bool` is a 1-byte MSL storage type, so bool vectors pack like any other:
// native `bool3` is padded to 4 bytes, `packed_bool3` is 3.
StructuredBuffer<bool3> flags;
RWStructuredBuffer<float> output;

// An explicit `ScalarDataLayout` argument requests the natural layout Metal
// device buffers already use, so it produces the same packed storage.
StructuredBuffer<float3, ScalarDataLayout> positionsScalar;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3 p = positions[tid.x] + positionsScalar[tid.x];
    float4 c = colors[tid.x];
    float2 uv = uvs[tid.x];
    Wrapped w = wrapped[tid.x];
    bool3 f = flags[tid.x];
    output[tid.x] = p.x + p.y + p.z + c.w + uv.y + w.scale + w.v.x + (f.y ? 1.0 : 0.0);
}

// Vector struct fields are lowered to packed vectors too.
// METAL: struct Wrapped_natural
// METAL: float scale
// METAL: packed_float4 v

// METAL-NOT: {{[ (,]float3 device\* positions}}
// METAL-DAG: packed_float3 device* positions
// METAL-DAG: packed_float4 device* colors
// METAL-DAG: packed_float2 device* uvs
// METAL-DAG: packed_float3 device* positionsScalar
// METAL-DAG: packed_bool3 device* flags
// METAL-NOT: {{[ (,]float3 device\* positions}}
