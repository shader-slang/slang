//TEST:COMPARE_COMPUTE(targets=vk+metal+cpu+wgpu+llvm, filecheck-buffer=BUF):
//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=BUF):-render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=cuda, filecheck-buffer=BUF):-g0

import glsl;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//
// Tests GLSL array.length() syntax.
//

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int myArray[4] = {4, 3, 2, 1};
    outputBuffer[0] = myArray[0];
    outputBuffer[1] = myArray[1];
    outputBuffer[2] = myArray[2];
    outputBuffer[3] = myArray[3];
    outputBuffer[4] = myArray.length();

    // BUF: 4
    // BUF-NEXT: 3
    // BUF-NEXT: 2
    // BUF-NEXT: 1
    // BUF-NEXT: 4
}



