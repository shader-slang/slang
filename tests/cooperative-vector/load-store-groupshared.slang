//TEST:COMPARE_COMPUTE(targets=vk, filecheck-buffer=CHECK):-render-feature cooperative-vector
//TEST:COMPARE_COMPUTE(targets=dx12, filecheck-buffer=CHECK):-render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xdxc -Vd
//TEST:COMPARE_COMPUTE(targets=cuda+cpu, filecheck-buffer=CHECK):-output-using-type

//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_9 -entry computeMain
//DXIL: result code = 0

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5

//TEST_INPUT:ubuffer(data=[1 2 3 4 5]):name=input
RWByteAddressBuffer input;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0]):out,name=outputBuffer
RWStructuredBuffer<uint32_t> outputBuffer;

groupshared uint32_t[5] temp;

[numthreads(1, 1, 1)]
void computeMain()
{
    let vec = coopVecLoad<5, uint32_t>(input);
    vec.store(temp);
    let result = coopVecLoadGroupshared<5>(temp);
    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];
}
