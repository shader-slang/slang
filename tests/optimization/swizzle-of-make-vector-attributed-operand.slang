// A `RWTexture2D<unorm float>` read has the attributed type `unorm float`. The attribute
// doesn't change which lane of the vector constructor the value supplies, so a swizzle of
// `float2(tex[id], 0.0)` or `float3(t, t, 2.0)` still resolves to the constructor's operands,
// through both the swizzle and the `getElement` fold.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute -O0

RWTexture2D<unorm float> tex;
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
}
