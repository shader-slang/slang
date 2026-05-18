// Verifies that explicit coverage bindings are checked against
// descriptor slots occupied by `ParameterBlock<T>` members, not just
// top-level global parameters.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage-binding 0 0

struct Params
{
    RWStructuredBuffer<uint> data;
};

[[vk::binding(0, 0)]]
ParameterBlock<Params> params;
//CHECK:               ^^^^^^ error E45101

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        params.data[0] = 1;
    }
}
