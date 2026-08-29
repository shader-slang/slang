//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

[format("r16f")]
RWTexture1D<float> scalarSurface;
[format("rg16f")]
RWTexture1D<float2> vector2Surface;
[format("rgba16f")]
RWTexture1D<float4> vector4Surface;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    scalarSurface[index] = float(index);
    vector2Surface[index] = float2(index, index + 1);
    vector4Surface[index] = float4(index, index + 1, index + 2, index + 3);

    float4 v4 = vector4Surface[index];
    float2 v2 = vector2Surface[index];
    float v = scalarSurface[index];
    outputBuffer[index] = v4.x + v2.x + v;
}

// CHECK: sust.p.1d.b32.zero
// CHECK: sust.p.1d.v2.b32.zero
// CHECK: sust.p.1d.v4.b32.zero
// CHECK: suld.b.1d.v4.b16.zero
// CHECK: suld.b.1d.v2.b16.zero
// CHECK: suld.b.1d.b16.zero
