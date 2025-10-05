//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-llvm -compute -shaderobj

import "ambiguous-extern-export-lib1.slang";
import "ambiguous-extern-export-lib2.slang";

export static const int call_data_len = 6;
export static const int call_group_vector[call_data_len] = {1,2,3,4,5,6};

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    initCallId1();
    initCallId2();

    for (int i = 0; i < call_data_len; i++)
    {
        outputBuffer[i] = call_id_1[i] + call_id_2[i];
    }
    // BUFFER: 2
    // BUFFER-NEXT: 4
    // BUFFER-NEXT: 6
    // BUFFER-NEXT: 8
    // BUFFER-NEXT: A
    // BUFFER-NEXT: C
}
