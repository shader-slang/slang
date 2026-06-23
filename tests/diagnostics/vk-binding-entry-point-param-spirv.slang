// Reproduces shader-slang/slang#9216: vk::binding on entry-point parameters is ignored,
// so SPIR-V compilation should warn instead of silently assigning default bindings.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -entry main -target spirv

[numthreads(32, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(/* binding */ 0, /* set */ 1)]]
    StructuredBuffer<float> a,
//CHECK:                    ^ modifier on entry point parameter is unsupported
//CHECK:                    ^ attribute '[[vk::binding(...)]]' on parameter 'a' is unsupported on entry point parameters and will be ignored
    [[vk::binding(/* binding */ 3, /* set */ 2)]]
    RWStructuredBuffer<float> b,
//CHECK:                      ^ modifier on entry point parameter is unsupported
//CHECK:                      ^ attribute '[[vk::binding(...)]]' on parameter 'b' is unsupported on entry point parameters and will be ignored
)
{
    b[0] = a[0];
}
