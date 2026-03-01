//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

RWStructuredBuffer<float> buffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // CHECK: ([[# @LINE+1]]): error 30086
    float* p = &buffer[threadId.x];
}
