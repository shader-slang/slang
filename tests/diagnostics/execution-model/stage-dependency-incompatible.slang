// Test that using SV_Depth (fragment-only) in a compute shader is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(out float depth : SV_Depth)
//CHECK:                   ^^^^^ system value semantic 'SV_Depth' cannot be used as output in 'compute' shader stage
{
    depth = 0.5;
}
