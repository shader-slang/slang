// Test that SV_DispatchThreadID with a float type produces an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain

[numthreads(4, 1, 1)]
void computeMain(float3 tid : SV_DispatchThreadID)
/*CHECK:
                        ^^^ invalid SV_DispatchThreadID type
                        ^^^ parameter with SV_DispatchThreadID must be either scalar or vector (1 to 3) of uint/int
*/
{
}
