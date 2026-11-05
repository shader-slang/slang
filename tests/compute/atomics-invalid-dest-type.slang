// atomics-buffer.slang

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target metal -stage compute -entry computeMain

//CHECK: atomic operation on non-scalar texture

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
