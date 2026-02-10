//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[1.0 1.0 1.0 1.0], stride=4):name=gInput
RWStructuredBuffer<float> gInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<float> gOutput;

// Note that COUNT-5 will fail but COUNT-3 wouldn't fail.
//CHK-COUNT-3:1

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    gOutput[tid] = gInput[tid];
}
