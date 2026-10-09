// Metal buffer lowering loads a `float4` element as a `packed_float4` and converts it back
// with a one-operand vector constructor, `float4(packed)`. The constructor peepholes resolve
// lanes only through scalar or `vector<T,N>` operands, so they conservatively leave that
// conversion in place instead of folding a swizzle or index into the packed value.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute

StructuredBuffer<float4> colors;
RWStructuredBuffer<float> output;
RWStructuredBuffer<float2> output2;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    // CHECK: (float4(*(colors{{[_0-9]*}}+i{{[_0-9]*}})) ).x;
    output[i] = colors[i].x;
    // CHECK: (float4(*(colors{{[_0-9]*}}+i{{[_0-9]*}})) ).xx
    output2[i] = colors[i].xx;
    // CHECK: (float4(*(colors{{[_0-9]*}}+i{{[_0-9]*}})) )[int(0)];
    output[i + 1] = colors[i][0];
}
