//TEST:SIMPLE(filecheck=CHECK):-target hlsl -profile vs_5_0 -entry MainVs

// Regression test for https://github.com/shader-slang/slang/issues/10328
// Verify that HLSL semantic indices are preserved correctly (e.g., TEXCOORD1
// should not become TEXCOORD10).

// CHECK: TEXCOORD0
// CHECK: TEXCOORD1
// CHECK-NOT: TEXCOORD00
// CHECK-NOT: TEXCOORD10

struct VS_INPUT
{
    float4 vPositionSs : POSITION;
    float4 vTex        : TEXCOORD0;
    float4 vColor      : TEXCOORD1;
};

struct PS_INPUT
{
    float4 vTex      : TEXCOORD0;
    float4 vColor    : TEXCOORD1;
    float4 vPosition : SV_Position;
};

PS_INPUT MainVs(VS_INPUT i)
{
    PS_INPUT o;
    o.vPosition = i.vPositionSs;
    o.vPosition.z = 0;
    o.vTex = i.vTex;
    o.vColor = i.vColor;
    return o;
}
