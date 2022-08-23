// atomic-float-byte-address-buffer-cross.slang

//TEST:CROSS_COMPILE: -profile cs_6_5 -entry computeMain -target spirv-assembly
// We can't do this test, because it relies on nvAPI
//DISABLE_TEST:CROSS_COMPILE: -profile cs_6_5 -entry computeMain -target dxil

//TEST_INPUT:ubuffer(data=[0.1 0.2 0.3 0.4]):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

//TEST_INPUT:ubuffer(data=[0.7 0.5 0.2 0.6]):name=anotherBuffer
RWStructuredBuffer<float> anotherBuffer;

[numthreads(16, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    int idx = int((tid & 3) ^ (tid >> 2)); 

    const float delta = anotherBuffer[idx & 3];
    
    float previousValue = 0;
    outputBuffer.InterlockedAddF32((idx << 2), 1.0f, previousValue);
    
    // The sum of values in anotherBuffer should also be added
    int anotherIdx = int(tid >> 2);
    outputBuffer.InterlockedAddF32(anotherIdx << 2, delta);
}