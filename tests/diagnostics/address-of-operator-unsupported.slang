//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute

RWStructuredBuffer<float> buffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // CHECK: error[E31207]: use of removed declaration
    // CHECK: the '&' operator for taking addresses is no longer supported in Slang
    float* p = &buffer[threadId.x];
}
