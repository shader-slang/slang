//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib

// METALLIB-NOT: error :
// METALLIB: @computeMain

// Test that we can compile texture store operations with 2-component vectors
// without segfaulting. This reproduces the issue where legalizeImageStoreValue
// would crash when trying to expand a float2 to float4 for Metal texture writes.

RWTexture2D<float2> output;

[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // This should expand float2(1,2) to float4(1,2,0,0)
    // METAL: {{.*}}float4(1.0, 2.0,
    output.Store(dispatchThreadID.xy, float2(1, 2));
} 
