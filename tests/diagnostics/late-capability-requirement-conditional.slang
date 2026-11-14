// Test for a compile-time conditional capability requirement, success case: unmet capability compiled out

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-ERR):-stage compute -entry main -target hlsl -profile sm_5_0 -restrictive-capability-check
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-WARN):-stage compute -entry main -target hlsl -profile sm_5_0

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (true)
    {
        __requireCapability(shadermemorycontrol); // sm_5_0 capability
    }
    else
    {
        __requireCapability(cooperative_vector); // sm_6_9 capability, compiled out
    }
}
