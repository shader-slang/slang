// Test for a compile-time conditional capability requirement, failure case: unmet capability not compiled out

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-ERR):-stage compute -entry main -target hlsl -profile sm_5_0 -restrictive-capability-check -DERROR=1
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-WARN):-stage compute -entry main -target hlsl -profile sm_5_0 -DERROR=0

#if ERROR

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECK-ERR:
     ^^^^ entry point uses capabilities not in specified profile
     ^^^^ entry point 'main' uses capabilities that are not part of the specified profile 'sm_5_0'. Missing capabilities are: 'sm_6_9'
*/
{
    if (false)
    {
        __requireCapability(shadermemorycontrol); // sm_5_0 capability, compiled out
    }
    else
    {
        __requireCapability(cooperative_vector); // sm_6_9 capability
/*CHECK-ERR:
        ^^^^^^^^^^^^^^^^^^^ see call to '__requireCapability'
*/
    }
}

#else

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECK-WARN:
     ^^^^ profile implicitly upgraded
     ^^^^ entry point 'main' uses additional capabilities that are not part of the specified profile 'sm_5_0'. The profile setting is automatically updated to include these capabilities: 'sm_6_9'
*/
{
    if (false)
    {
        __requireCapability(shadermemorycontrol); // sm_5_0 capability, compiled out
    }
    else
    {
        __requireCapability(cooperative_vector); // sm_6_9 capability
/*CHECK-WARN:
        ^^^^^^^^^^^^^^^^^^^ see call to '__requireCapability'
*/
    }
}

#endif
