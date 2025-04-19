// Test that texture subscripting in CUDA targets is properly diagnosed with the correct error message.
//TEST:SIMPLE(filecheck=CHECK): -target cuda

Texture1D<float> tex1D;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    
    float value = tex1D[idx];
    
    outputBuffer[0] = value;
}
// CHECK: 
