//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-glsl -render-feature cooperative-vector -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-target spirv-asm -emit-spirv-via-glsl -render-feature cooperative-vector -output-using-type

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4),name=inputBuffer
ByteAddressBuffer inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var result = CoopVec<int32_t, 4>();
    
    // Load data using simple method for GLSL compatibility
    for(int i = 0; i < 4; ++i)
        result[i] = inputBuffer.LoadByteOffset<int32_t>(i * 4);

    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];
}