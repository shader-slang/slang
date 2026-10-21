// A `TextureBuffer` whose element mixes ordinary data with resources is split by resource
// legalization into a data-only texture buffer plus one parameter per resource
// (shader-slang/slang#13789). The data-only part binds only as a texture, like a `TextureBuffer`
// that never held resources, and does not also take the buffer slot that `usedBuf` gets.

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry main

struct P
{
    float4 tint;
    StructuredBuffer<float4> usedBuf;
    StructuredBuffer<float4> unusedBuf;
};

RWStructuredBuffer<float4> Output;
TextureBuffer<P> gP;

// CHECK: texture_buffer<{{[^>]*}}> gP_{{[0-9]+}} {{\[\[texture\(0\)\]\]}},
// CHECK-SAME: gP_usedBuf_{{[0-9]+}} {{\[\[buffer\(1\)\]\]}}
[numthreads(64, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    Output[tid.x] = gP.usedBuf[tid.x] * gP.tint;
}
