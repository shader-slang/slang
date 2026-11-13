// An entry point declared `__extern_cpp` without a body is an unresolved symbol on every target.
// `__extern_cpp` lets an ordinary function bind to a C/C++ definition outside the Slang module, but
// every target builds an entry point from its Slang body, so the body must be present.
// See https://github.com/shader-slang/slang/issues/14200
//
// The front end marks `NoBody` as an entry point both for `[shader]` and for `-entry` alone
// (NO_SHADER_ATTRIBUTE), so every row here is reported by the check in
// `isFunctionDefinedOrImported`. In the CALLED_FIRST rows `Caller` also calls `NoBody`; C++ source
// without `-o` links each entry point on its own, so the TWICE row reports `NoBody` in both links,
// once as a callee that is still marked as an entry point. The check in `specializeIRForEntryPoint`,
// for an entry point the front end did not mark, is covered by
// unit-test-extern-cpp-entry-point-without-body.cpp. Two identical "see declaration" notes
// belong to each E45001 report; they do not mean the error was emitted twice.

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
