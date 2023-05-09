//TEST:SIMPLE(filecheck=CHECK):-target hlsl -compute -profile sm_5_0 -output-using-type

// Test that we can specialize a generic method called through a dynamic interface.

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<float> gOutputBuffer;

float test()
{
    return gOutputBuffer[0];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    test();
}

// CHECK: void computeMain
// CHECK-NOT: test
// CHECK: }

