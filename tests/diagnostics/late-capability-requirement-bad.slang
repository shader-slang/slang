// Test for a bad capability requirement: bad capability name triggers an error
// regardless of restrictive/non-restrictive/ignore mode and whether it's compiled out

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry main -target hlsl -profile sm_5_0 -restrictive-capability-check
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry main -target hlsl -profile sm_5_0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry main -target hlsl -profile sm_5_0 -ignore-capabilities

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (false)
    {
        __requireCapability(ThisIsANonExistingCapability);
//CHECK:                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ unknown capability
//CHECK:                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ unknown capability name 'ThisIsANonExistingCapability'.
    }
}
