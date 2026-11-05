// Two entry points that an emitter writes under the same symbol make output that the downstream
// compiler or validator rejects (shader-slang/slang#14053), so E38016 reports them: overloads and
// same-named functions in different namespaces both collide. The HLSL and C++ outputs collide too,
// although both languages allow overloads: a DXIL library exports entry points under unmangled
// names, and the CPU entry-point wrappers have C linkage.
//
// Only entry points that share an output can collide. Without `-o`, slangc writes all entry points
// into one SPIR-V, CUDA or Metal output, but writes WGSL, HLSL and C++ one entry point at a time,
// so those lanes pass `-whole-program`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -whole-program
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -profile lib_6_6 -whole-program
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -whole-program
//DIAGNOSTIC_TEST:SIMPLE(filecheck=VIA_GLSL,exit-code=-1):-target spirv-asm -emit-spirv-via-glsl -whole-program

// The names are compared under the rules of the format that the emitter writes, which is GLSL
// here, so the compute overloads that collide on SPIR-V give no E38016; glslang rejects the
// whole-program GLSL instead.
// VIA_GLSL-NOT: E38016
// VIA_GLSL: E00101
// VIA_GLSL-NOT: E38016

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void Count(uniform uint a, uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^^^^^ the other entry point emitted as 'Count' is here
*/
{
    outBuf[0] = a;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void Count(uniform float a, uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^^^^^ more than one entry point would be emitted as 'Count' for target
     ^^^^^ this entry point would also be emitted as 'Count'
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
     ^ the other entry point emitted as 'K' is here
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
     ^ more than one entry point would be emitted as 'K' for target
     ^ this entry point would also be emitted as 'K'
*/
{
    outBuf[1] = 2;
}
}
