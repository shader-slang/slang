//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -profile ps_5_0 -entry ps_main

// Regression for #11018.  When a call with a `constexpr` parameter receives
// an argument whose value ultimately depends on a runtime (cbuffer) value
// flowing through a non-unrollable for-loop, the compiler used to emit E40012
// five times per call: once at the real offending argument and four
// misleading times at the for-loop init / ++y / ++x syntax.  With the fix,
// only the error at the real argument should fire; the for-loop syntax
// should NOT be flagged.

Texture2D<uint2> g_selectedObjects;

cbuffer SelectionCB
{
    int g_width;
};

struct VS2PSFSVertex
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

[shader("pixel")]
float4 ps_main(VS2PSFSVertex i) : SV_Target0
{
    const int width = g_width;
    const int3 texel = int3(int2(i.uv * 4.0), 0);
    uint4 acc = 0;
    [loop]
    for (int y = -width; y <= width; ++y)
    {
        [loop]
        for (int x = -width; x <= width; ++x)
        {
            acc += g_selectedObjects.Load(texel, int2(x, y)).r;
//CHECK:                                             ^ expected a compile-time constant
        }
    }
    return (float4)acc;
}
