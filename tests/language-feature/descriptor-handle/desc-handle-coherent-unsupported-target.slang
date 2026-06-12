//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute

// CHECK: error[E55204]: unsupported intrinsic operation

coherent RWStructuredBuffer<uint> coherentBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    coherentBuffer[dispatchThreadID.x] = 0;
}
