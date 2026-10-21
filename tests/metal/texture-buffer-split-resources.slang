// A `TextureBuffer` whose element mixes ordinary data with resources is split by resource
// legalization into a data-only texture buffer plus one parameter per resource
// (shader-slang/slang#13789). The data-only part must bind only as a texture, like a
// `TextureBuffer` that never held resources; `[[buffer(1)]]` belongs to `usedBuf`, after `Output`
// takes buffer 0.

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry main

struct P
{
    float4 tint;
    StructuredBuffer<float4> usedBuf;
    StructuredBuffer<float4> unusedBuf;
};

RWStructuredBuffer<float4> Output;
TextureBuffer<P> gP;

// The parameter `gP` is followed directly by its `[[texture(0)]]` attribute and then the next
// parameter, so a `[[buffer(...)]]` on `gP` fails the match.
// CHECK: texture_buffer<{{[^>]*}}> gP_{{[0-9]+}} {{\[\[texture\(0\)\]\]}},
// CHECK-SAME: gP_usedBuf_{{[0-9]+}} {{\[\[buffer\(1\)\]\]}}
[numthreads(64, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    Output[tid.x] = gP.usedBuf[tid.x] * gP.tint;
}
