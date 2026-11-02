// An entry point declared `__extern_cpp` without a body is an unresolved symbol on every target.
// `__extern_cpp` lets an ordinary function bind to a C/C++ definition outside the Slang module, but
// every target builds an entry point from its Slang body, so the body must be present.
// See https://github.com/shader-slang/slang/issues/14200
//
// The entry point is reported however it is selected: by its `[shader]` attribute, by `-entry`
// alone (NO_SHADER_ATTRIBUTE), or when another entry point calls it (CALLED_FIRST). For C++ source
// without `-o`, each entry point is linked on its own, so `NoBody` is reported in both links: as
// the entry point, and as a callee of `Caller` that is still marked as an entry point.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuh
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hpp
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry NoBody -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl -entry NoBody -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry NoBody -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -entry NoBody -stage compute

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -DNO_SHADER_ATTRIBUTE -entry NoBody -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -DNO_SHADER_ATTRIBUTE -entry NoBody -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -DNO_SHADER_ATTRIBUTE -entry NoBody -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -DNO_SHADER_ATTRIBUTE -entry NoBody -stage compute

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -DCALLED_FIRST -entry Caller -stage compute -entry NoBody -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=TWICE):-target cpp -DCALLED_FIRST -entry Caller -stage compute -entry NoBody -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -DCALLED_FIRST -entry Caller -stage compute -entry NoBody -stage compute

#ifdef CALLED_FIRST
[shader("compute")]
[numthreads(1, 1, 1)]
void Caller(uniform float* p, uint3 tid : SV_DispatchThreadID)
{
    NoBody(p, tid);
}
#endif

#ifndef NO_SHADER_ATTRIBUTE
[shader("compute")]
#endif
[numthreads(1, 1, 1)]
__extern_cpp void NoBody(uniform float* p, uint3 tid : SV_DispatchThreadID);
//CHECK:          ^^^^^^ E45001
//CHECK:          ^^^^^^ unresolved external symbol 'NoBody'
//CHECK:          ^^^^^^ see declaration of 'NoBody'
//CHECK:          ^^^^^^ see declaration of 'NoBody'
//TWICE: E45001
//TWICE: unresolved external symbol 'NoBody'
//TWICE: E45001
//TWICE: unresolved external symbol 'NoBody'
//TWICE: see declaration of 'NoBody'
//TWICE: see declaration of 'NoBody'
//TWICE: see declaration of 'NoBody'
//TWICE: see declaration of 'NoBody'
