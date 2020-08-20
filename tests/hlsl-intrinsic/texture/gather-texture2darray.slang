// gather-texture2darray.slang

//TEST:CROSS_COMPILE: -target dxbc -profile sm_5_1 -entry main -stage compute

// Test gathering from a `Texture2DArray`

Texture2DArray<uint> t;
SamplerState s;
RWBuffer<uint4> b;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    b[tid.x] = t.Gather(s, tid);
}
