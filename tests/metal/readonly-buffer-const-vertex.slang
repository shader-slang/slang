// A vertex function that returns a value may not have writable buffers: Apple's compiler warns
// "writable resources in non-void vertex function" for each one. Read-only buffers are emitted as
// `const device` pointers, so they are not writable resources (#14154).

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry main -stage vertex
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry main -stage vertex

// METALLIB: @main

StructuredBuffer<float> scales;
ByteAddressBuffer raw;

struct VSOutput
{
    float4 position : SV_Position;
};

// CHECK: {{\[\[}}vertex{{\]\]}} {{.*}} main{{.*}}(uint {{.*}} {{\[\[}}vertex_id{{\]\]}}, const float device* scales_{{[0-9]+}} {{\[\[}}buffer(0){{\]\]}}, const uint32_t device* raw_{{[0-9]+}} {{\[\[}}buffer(1){{\]\]}})
[shader("vertex")]
VSOutput main(uint vid: SV_VertexID)
{
    VSOutput o;
    o.position = float4(float(vid), 0, 0, 1) * scales[vid] + asfloat(raw.Load(vid * 4));
    return o;
}
