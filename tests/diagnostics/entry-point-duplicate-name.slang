// Two entry points that the emitter writes under the same name make output that the downstream
// compiler or validator rejects (shader-slang/slang#14053). The linked program is checked for
// such collisions: overloads and same-named functions in different namespaces both collide.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -whole-program
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -profile lib_6_6 -whole-program
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -whole-program

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void Count(uniform uint a, uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^^^^^ see the other entry point named 'Count'
*/
{
    outBuf[0] = a;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void Count(uniform float a, uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^^^^^ more than one entry point is named 'Count' in the output for target
     ^^^^^ entry point named 'Count' conflicts with another entry point
*/
{
    outBuf[0] = uint(a);
}

namespace A
{
[shader("compute")]
[numthreads(1, 1, 1)]
void K(uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^ see the other entry point named 'K'
*/
{
    outBuf[1] = 1;
}
}

namespace B
{
[shader("compute")]
[numthreads(1, 1, 1)]
void K(uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^ more than one entry point is named 'K' in the output for target
     ^ entry point named 'K' conflicts with another entry point
*/
{
    outBuf[1] = 2;
}
}
