//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target metal -entry main -stage compute

// shader-slang/slang#13737: the `$q` strings for `Load` on a combined texture-sampler read the
// coordinate from call operand 2, because operand 1 is the split-out sampler. The emitter decides
// whether to fold an argument into the call by counting its uses in the target string, so that
// count must use the same operand mapping. If it counted `($1)` as the sampler, the coordinate
// would look unused and be folded into every `($1)` of the next `Load`, and the expression would
// grow exponentially with each chained `Load`.

Sampler2DArray<int4> u_tex;
RWStructuredBuffer<int4> outBuf;

// Five chained loads must stay five texture reads.
// CHECK-COUNT-5: {{textureLoad|\.read}}(
// CHECK-NOT: {{textureLoad|\.read}}(

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 id: SV_DispatchThreadID)
{
    int4 c = int4(id.xyzz);
    c = u_tex.Load(c);
    c = u_tex.Load(c);
    c = u_tex.Load(c);
    c = u_tex.Load(c);
    c = u_tex.Load(c);
    outBuf[0] = c;
}
