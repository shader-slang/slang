// atomic-float-byte-address-buffer-cross.slang

//TEST:SIMPLE(filecheck=CHECK_SPV): -stage compute -entry computeMain -target spirv
//TEST:SIMPLE(filecheck=CHECK_GLSL): -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_HLSL): -stage compute -entry computeMain -target hlsl

RWByteAddressBuffer outputBuffer;

RWStructuredBuffer<float> anotherBuffer;

// CHECK_SPV: OpAtomicFAddEXT
// CHECK_SPV: OpAtomicFAddEXT
// CHECK_GLSL: atomicAdd
// CHECK_GLSL: atomicAdd
// CHECK_HLSL: NvInterlockedAddFp32
// CHECK_HLSL: NvInterlockedAddFp32

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