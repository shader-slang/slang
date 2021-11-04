// Main entry-point. Applies the transformation encoded by `transformer`
// to all elements in `buffer`.

uniform RWStructuredBuffer<float> buffer;

[shader("compute")]
[numthreads(4,1,1)]
void computeMain(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID)
{
    var input = buffer[sv_dispatchThreadID.x];
    buffer[sv_dispatchThreadID.x] = input + 1.0f;
}
