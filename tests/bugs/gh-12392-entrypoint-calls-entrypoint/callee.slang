// Companion module for `outer-uniform-callee.slang`: a compute entry point whose parameters are
// `uniform` resources and a system-value input.
module callee;

[shader("compute")]
[numthreads(64, 1, 1)]
public void inner(uniform RWStructuredBuffer<uint> b, uniform uint n, uint3 tid: SV_DispatchThreadID)
{
    if (tid.x < n)
        b[tid.x] = tid.x;
}
