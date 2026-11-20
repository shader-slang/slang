//TEST:COMPARE_COMPUTE(targets=vk, filecheck-buffer=CHECK):-output-using-type -render-feature bindless
//TEST:COMPARE_COMPUTE(targets=dx12, filecheck-buffer=CHECK):-output-using-type -profile sm_6_6 -render-feature bindless
//TEST:COMPARE_COMPUTE(targets=metal, filecheck-buffer=CHECK):-output-using-type

// CHECK: type: uint
// CHECK-NEXT: 42

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:ubuffer(data=[42], stride=4):name=input
uniform RWStructuredBuffer<uint>.Handle input;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Just read one value
    outputBuffer[0] = input[0];
}
