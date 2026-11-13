// An entry point declared with a target intrinsic and no body is an unresolved symbol on every
// target. An intrinsic stands in for the body of an ordinary function, but every target builds an
// entry point from its Slang body, so the body must be present.
// See https://github.com/shader-slang/slang/issues/14200

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry NoBody -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
__target_intrinsic(hlsl, "noBody()")
__target_intrinsic(cuda, "noBody()")
__target_intrinsic(cpp, "noBody()")
void NoBody(uniform float* p, uint3 tid : SV_DispatchThreadID);
/*CHECK:
     ^^^^^^ E45001
     ^^^^^^ unresolved external symbol 'NoBody'
     ^^^^^^ see declaration of 'NoBody'
     ^^^^^^ see declaration of 'NoBody'
*/
