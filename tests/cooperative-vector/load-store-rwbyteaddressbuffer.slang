//TEST:COMPARE_COMPUTE(targets=vk, filecheck-buffer=CHECK):-render-feature cooperative-vector
//TEST:COMPARE_COMPUTE(targets=dx12, filecheck-buffer=CHECK):-render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xdxc -Vd
//TEST:COMPARE_COMPUTE(targets=cuda+cpu, filecheck-buffer=CHECK):

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
RWByteAddressBuffer outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let result = coopVecLoad<5, uint32_t>(input);
    result.store(outputBuffer);
}
