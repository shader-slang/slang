// Test for a missing capability: error, warn, or no diagnostics based on commandline flags

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-ERR):-stage compute -entry main -target hlsl -profile sm_5_0 -restrictive-capability-check -DERROR=1 -DWARN=0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-WARN):-stage compute -entry main -target hlsl -profile sm_5_0 -DERROR=0 -DWARN=1
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-NOERR):-stage compute -entry main -target hlsl -profile sm_5_0 -ignore-capabilities -DERROR=0 -DWARN=0

#if ERROR

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECK-ERR:
     ^^^^ entry point uses capabilities not in specified profile
     ^^^^ entry point 'main' uses capabilities that are not part of the specified profile 'sm_5_0'. Missing capabilities are: 'sm_6_9'
*/
{
    __requireCapability(cooperative_vector);
/*CHECK-ERR:
    ^^^^^^^^^^^^^^^^^^^ see call to '__requireCapability'
*/
}

#elif WARN

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
/*CHECK-WARN:
     ^^^^ profile implicitly upgraded
     ^^^^ entry point 'main' uses additional capabilities that are not part of the specified profile 'sm_5_0'. The profile setting is automatically updated to include these capabilities: 'sm_6_9'
*/
{
    __requireCapability(cooperative_vector);
/*CHECK-WARN:
    ^^^^^^^^^^^^^^^^^^^ see call to '__requireCapability'
*/
}

#else

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    __requireCapability(cooperative_vector);
}

#endif
