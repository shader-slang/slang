//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu

//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_9 -entry computeMain
//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_9 -entry computeMain -DRWBAB
//DXIL: result code = 0

// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

//TEST_INPUT:ubuffer(data=[4 3 5 7], stride=4),name=input1
//TEST_INPUT:ubuffer(data=[2 3 4 5], stride=4),name=input2

#if defined(RWBAB)
RWByteAddressBuffer input1;
RWByteAddressBuffer input2;
#else
ByteAddressBuffer input1;
ByteAddressBuffer input2;
#endif

[numthreads(1, 1, 1)]
void computeMain()
{
    let vec1 = coopVecLoad<4, int32_t>(input1);
    let vec2 = coopVecLoad<4, int32_t>(input2);

    let vec3 = CoopVec<float, 4>(vec1);
    let vec4 = CoopVec<float, 4>(vec2);

    let vec5 = CoopVec<uint, 4>(vec1);
    let vec6 = CoopVec<uint, 4>(vec2);

    let result = vec1 % vec2;
    let result2 = CoopVec<int, 4>(vec3 % vec4);
    let result3 = CoopVec<int, 4>(vec5 % vec6);

    result.store(outputBuffer);
    result2.store(outputBuffer, 16);
    result3.store(outputBuffer, 32);
}
