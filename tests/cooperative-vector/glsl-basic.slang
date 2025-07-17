//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-target glsl -stage compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-target spirv-asm -emit-spirv-via-glsl -stage compute -output-using-type

// CHECK: type: int32_t
// CHECK-NEXT: 15
// CHECK-NEXT: 15
// CHECK-NEXT: 15
// CHECK-NEXT: 15

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var result : CoopVec<int32_t, 4>;
    result.fill(10);
    
    var other : CoopVec<int32_t, 4>;
    other.fill(5);
    
    var sum = result.add(other);
    
    for(int i = 0; i < sum.getCount(); ++i)
        outputBuffer[i] = sum[i];
}