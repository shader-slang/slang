//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute

// CHECK: warning[E41016]: use of uninitialized variable
// CHECK-COUNT-2: error[E55204]: unsupported intrinsic operation

coherent RWStructuredBuffer<uint> coherentBuffer;

uint useCoherentHandleType(DescriptorHandle<coherent RWStructuredBuffer<uint>> coherentHandle)
{
    return 0;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Keep the local handle live so unsupported targets diagnose the type-attribute path.
    DescriptorHandle<coherent RWStructuredBuffer<uint>> coherentHandle;
    uint value = useCoherentHandleType(coherentHandle);
    coherentBuffer[dispatchThreadID.x] = 0;
}
