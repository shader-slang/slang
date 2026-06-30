// Test that system value semantics cannot be used outside their valid stages.
// Case 1: SV_Depth (fragment-only) used as output in a compute shader.
// Case 2: SV_GroupID (compute-only) used as input in a fragment shader.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1):-target spirv -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-target hlsl -entry fragMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(out float depth : SV_Depth)
//CHECK1:                  ^^^^^ system value semantic 'SV_Depth' cannot be used as output in 'compute' shader stage
{
    depth = 0.5;
}

//CHECK2: E30702
[shader("fragment")]
float4 fragMain(uint3 gid : SV_GroupID) : SV_Target
/*CHECK2:
                      ^^^
*/
{
    return float4(float(gid.x), 0.0, 0.0, 1.0);
}
