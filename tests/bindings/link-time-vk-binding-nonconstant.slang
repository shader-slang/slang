// A `vk::binding` argument that is not a constant expression at all must keep its
// diagnostic: the link-time checker runs with a null diagnostic sink, so a present
// argument that fails the check is diagnosed here and the attribute is dropped — the
// argument must never be treated like an unspecified one (that would silently bind at
// index 0). The double emission is expected: the attribute validates on both the
// modifier-check and layout passes.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -entry main -stage compute

int f()
{
    return 3;
}

[[vk::binding(f(), 1)]]
/*CHECK:
               ^ expression does not evaluate to a compile-time constant
               ^ expression does not evaluate to a compile-time constant
*/
RWStructuredBuffer<uint> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    result[threadID.x] = threadID.x;
}
