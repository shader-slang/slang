// Test that using a compute-only system value in a fragment shader
// produces a stage-incompatible diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry fragMain

//CHECK: E30702
[shader("fragment")]
float4 fragMain(uint3 gid : SV_GroupID) : SV_Target
{
    return float4(float(gid.x), 0.0, 0.0, 1.0);
}
/*CHECK:
                      ^^^
*/
