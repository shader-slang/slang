//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main

//CHECK: E39017
[shader("intersection")]
void main(out float3 x : MY_SEMANTIC)
//CHECK:             ^ the 'intersection' stage does not support `out` or `inout` entry point parameters
{
    x = float3(0.0);
}
