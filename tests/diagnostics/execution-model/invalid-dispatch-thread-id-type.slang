// Test that wrong types for SV_DispatchThreadID produce error E38024.
// computeMain1: float3 (wrong element type)
// computeMain2: uint4 (wrong dimension)

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain1
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-target hlsl -stage compute -entry computeMain2

[numthreads(1,1,1)]
void computeMain1(float3 tid : SV_DispatchThreadID)
//CHECK:                 ^^^ invalid SV_DispatchThreadID type
//CHECK:                 ^^^ parameter with SV_DispatchThreadID must be either scalar or vector (1 to 3) of uint/int but is vector<float,3>
{}

[numthreads(1,1,1)]
void computeMain2(uint4 tid : SV_DispatchThreadID)
//CHECK2:               ^^^ invalid SV_DispatchThreadID type
//CHECK2:               ^^^ parameter with SV_DispatchThreadID must be either scalar or vector (1 to 3) of uint/int but is vector<uint,4>
{}
