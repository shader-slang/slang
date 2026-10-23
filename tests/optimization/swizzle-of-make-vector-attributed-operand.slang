// A `RWTexture2D<unorm float>` read has the attributed type `unorm float`, and a
// `RWTexture2D<unorm float2>` read has the type `unorm float2`. The attribute doesn't change
// which lanes of the vector constructor the value supplies, so the lane lookup compares element
// types without attributes. A swizzle or index of `float2(tex[id], 0.0)`, `float3(t, t, 2.0)` or
// `float4(tex2[id], 0.0, 1.0)` therefore still resolves to the constructor's operands.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute -O0

RWTexture2D<unorm float> tex;
RWTexture2D<unorm float2> tex2;
RWStructuredBuffer<float> outb;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    // CHECK: outb{{[_0-9]*}}[int(0)] = 0.0f;
    outb[0] = float2(tex[id.xy], 0.0).y;
    // CHECK: outb{{[_0-9]*}}[int(1)] = 0.0f;
    outb[1] = float2(tex[id.xy], 0.0)[1];
    float t = tex[id.xy];
    // CHECK: outb{{[_0-9]*}}[int(2)] = 2.0f;
    outb[2] = float3(t, t, 2.0).z;
    // CHECK: outb{{[_0-9]*}}[int(3)] = 1.0f;
    outb[3] = float4(tex2[id.xy], 0.0, 1.0).w;
}
