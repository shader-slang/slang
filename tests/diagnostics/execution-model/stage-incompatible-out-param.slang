//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main

//CHECK: E41018
//CHECK: E39017
[shader("intersection")]
void main(out float3 x : MY_SEMANTIC) {}
/*CHECK:
                                       ^ returning without initializing out parameter
                     ^ the 'intersection' stage does not support `out` or `inout` entry point parameters
*/
