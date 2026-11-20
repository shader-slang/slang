//TEST:COMPARE_COMPUTE(targets=vk, filecheck-buffer=CHECK):-render-feature cooperative-vector -output-using-type
//TEST:COMPARE_COMPUTE(targets=dx12, filecheck-buffer=CHECK):-render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xdxc -Vd
//TEST:COMPARE_COMPUTE(targets=cuda+cpu, filecheck-buffer=CHECK):-output-using-type

//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_9 -entry computeMain
//DXIL: result code = 0

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 4.0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let result = CoopVec<float, 4>(int(1),half(2),uint(3),double(4));
    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];
}
