// An entry point declared `[DllImport]` without a body is an unresolved symbol on every target.
// `[DllImport]` lets an ordinary function bind to a function in a shared library, but every target
// builds an entry point from its Slang body, so the body must be present.
// See https://github.com/shader-slang/slang/issues/14200

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry NoBody -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
[DllImport("missing-library")]
void NoBody(uniform float* p, uint3 tid : SV_DispatchThreadID);
/*CHECK:
     ^^^^^^ E45001
     ^^^^^^ unresolved external symbol 'NoBody'
     ^^^^^^ see declaration of 'NoBody'
     ^^^^^^ see declaration of 'NoBody'
*/
