//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

struct Params
{
    RWStructuredBuffer<uint> data;
};

[numthreads(1, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(2, 1)]]
//CHECK:  ^^^^^^^ whole descriptor set requires binding 0
//CHECK:  ^^^^^^^ shader parameter 'params' consumes whole descriptor sets, so the binding must be in the form '[[vk::binding(0, ...)]]'; the non-zero binding '2' is not allowed
    ParameterBlock<Params> params)
{
    params.data[0] = 0;
}
