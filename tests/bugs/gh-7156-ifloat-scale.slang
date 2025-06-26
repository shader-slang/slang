//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-hlsl -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu -compute -output-using-type

// CHECK: 10

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Use of IFloat.scale used to cause an internal compiler error
func double_it<T: IFloat>(value: T) -> T {
    return value.scale(2.0);
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = double_it(float2(1.0, 2.0)).x + double_it(4.0);
}

