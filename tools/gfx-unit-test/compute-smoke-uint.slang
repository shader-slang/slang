// compute-smoke-uint.slang

// Main entry-point. Applies the transformation encoded by `transformer`
// to all elements in `buffer`.
[shader("compute")]
[numthreads(16,1,1)]
void computeMain(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<uint4> tex,
    uniform RWStructuredBuffer<uint4> buffer)
{
    buffer[sv_dispatchThreadID.x] = tex[uint2(sv_dispatchThreadID.x % 4, sv_dispatchThreadID.x / 4)];
}
