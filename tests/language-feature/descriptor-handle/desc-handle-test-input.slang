//TEST:COMPARE_COMPUTE(targets=vk, filecheck-buffer=CHECK):-output-using-type -render-feature bindless
//TEST:COMPARE_COMPUTE(targets=dx12, filecheck-buffer=CHECK):-output-using-type -profile sm_6_6 -render-feature bindless
//TEST:COMPARE_COMPUTE(targets=metal, filecheck-buffer=CHECK):-output-using-type

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 3.0

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0], stride=4):name=input
uniform RWStructuredBuffer<float>.Handle input;

[numthreads(1, 1, 1)]
void computeMain()
{
    for(int i = 0; i < 3; i++)
        outputBuffer[i] = input[i];
}
