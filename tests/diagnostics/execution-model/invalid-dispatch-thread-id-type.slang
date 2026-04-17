// Test that SV_DispatchThreadID with invalid types produces errors.
// Case 1: wrong element type (float3 — correct dimension, wrong type)
// Case 2: wrong dimension (uint4 — correct type, too many components)

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain1
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-target hlsl -stage compute -entry computeMain2

[numthreads(4, 1, 1)]
void computeMain1(float3 tid : SV_DispatchThreadID)
/*CHECK:
                         ^^^ invalid SV_DispatchThreadID type
                         ^^^ parameter with SV_DispatchThreadID must be either scalar or vector (1 to 3) of uint/int
*/
{
}

[numthreads(4, 1, 1)]
void computeMain2(uint4 tid : SV_DispatchThreadID)
/*CHECK2:
                        ^^^ invalid SV_DispatchThreadID type
                        ^^^ parameter with SV_DispatchThreadID must be either scalar or vector (1 to 3) of uint/int
*/
{
}
