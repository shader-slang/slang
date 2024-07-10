// atomics-buffer.slang

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

//CHECK: Atomic must be applied to a scalar texture or non-texture

RWBuffer<uint2> outputBuffer;

void test(uint val)
{
    uint originalValue;

	InterlockedAdd(outputBuffer[val][0], val, originalValue);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    test(tid);
}
