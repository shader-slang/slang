// Verifies that explicit coverage bindings are checked against
// descriptor slots occupied by entry-point uniform parameters. These
// parameters are flattened after coverage instrumentation, so the
// collision check must scan `IREntryPointLayout`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage-binding 0 0

struct Params
{
    RWStructuredBuffer<uint> data;
};

[numthreads(1, 1, 1)]
void computeMain(ParameterBlock<Params> params, uint3 tid : SV_DispatchThreadID)
//CHECK:                                ^^^^^^ `-trace-coverage-binding` collides with an existing parameter
{
    if (tid.x == 0)
    {
        params.data[0] = 1;
    }
}
