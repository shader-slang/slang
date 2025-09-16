//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-dx11
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-dx12
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-metal
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-wgpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-cpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-cuda

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=gInput
RWStructuredBuffer<float> gInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<int> gOutput;

//CHK:1
//CHK-NEXT:2
//CHK-NEXT:3
//CHK-NEXT:4

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    gOutput[tid] = int(gInput[tid]);
}
