// A body-less generic entry point is reported when cloning checks the generic itself. The entry
// point is found by the mangled name of the generic's specialization, so the post-clone check
// must look at the generic too. Two E45001 reports are expected here; a third would diagnose
// the specialization without a source location.
// See https://github.com/shader-slang/slang/issues/14200

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry NoBodyGeneric<float> -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry NoBodyGeneric<float> -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry NoBodyGeneric<float> -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void NoBodyGeneric<T>(uniform T* p, uint3 tid : SV_DispatchThreadID);
/*CHECK:
     ^^^^^^^^^^^^^ E45001
     ^^^^^^^^^^^^^ unresolved external symbol
     ^^^^^^^^^^^^^ see declaration of
     ^^^^^^^^^^^^^ see declaration of
     ^^^^^^^^^^^^^ E45001
     ^^^^^^^^^^^^^ unresolved external symbol 'NoBodyGeneric'
*/
