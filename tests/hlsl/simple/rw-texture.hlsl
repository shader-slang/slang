// rw-texture.hlsl

//TEST:COMPARE_HLSL:-no-mangle -profile ps_5_0 -entry main

// Ensure that we implement the `Load` operations on
// `RWTexture*` types with the correct signature.

#ifndef __SLANG__
#define C C_0
#define SV_Target SV_TARGET
#define u2 u2_0
#define u3 u3_0
#define t2 t2_0
#define t2a t2a_0
#define t3 t3_0
#endif


cbuffer C : register(b0)
{
    uint2 u2;
    uint3 u3;
};

RWTexture2D<float4>         t2  : register(u1);
RWTexture2DArray<float4>    t2a : register(u2);
RWTexture3D<float4>         t3  : register(u3);

float4 main() : SV_Target
{
    return t2.Load(u2)
        + t2a.Load(u3)
        + t3.Load(u3);
}
