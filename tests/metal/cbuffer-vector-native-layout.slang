//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -force-glsl-scalar-layout

// Constant buffers on Metal keep the native MSL layout (`float3` aligned and
// padded to 16 bytes), which is what reflection reports for them. Vectors in
// constant buffers must NOT be lowered to packed vectors, even though device
// buffers use packed (natural) layout by default and even when scalar layout
// is requested explicitly.

struct Params
{
    float3 a;
    float3 b;
    float4x4 m;
};

ConstantBuffer<Params> params;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = params.a.x + params.b.z + params.m[1][2];
}

// METAL-NOT: packed_
// METAL: struct Params
// METAL: float3 a
// METAL: float3 b
// METAL-NOT: packed_
