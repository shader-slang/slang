//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -force-glsl-scalar-layout

// Constant buffers keep the native MSL layout reflection reports for them
// (`float3` padded to 16 bytes); their vectors are NOT packed, even though
// device buffers pack by default and even with `-force-glsl-scalar-layout`.

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
