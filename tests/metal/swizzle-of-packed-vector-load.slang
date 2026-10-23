// Metal buffer lowering loads a `float4` element as a `packed_float4` and converts it back
// with a one-operand vector constructor, `float4(packed)`. An index reads its element directly
// from the packed value, but a swizzle keeps the conversion, because folding it would swizzle
// the packed value as if it were a `float4`.

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
    // CHECK: = (*(colors{{[_0-9]*}}+i{{[_0-9]*}}))[int(0)];
    output[i + 1] = colors[i][0];
}
