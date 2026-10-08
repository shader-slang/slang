//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(1, 0)]]
    uint3 tid : SV_DispatchThreadID)
//CHECK:  ^^^ modifier on entry point parameter is unsupported
//CHECK:  ^^^ attribute '[[vk::binding(...)]]' on parameter 'tid' is unsupported on entry point parameters and will be ignored
{
    output[tid.x] = 0;
}
