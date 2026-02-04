//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type -Xslang -DREPRO

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=gInput
RWStructuredBuffer<float> gInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<float> gOutput;

//CHK:1
//CHK-NEXT:2
//CHK-NEXT:3
//CHK-NEXT:4

struct P
{
        Texture2D.Handle                t;
};

ParameterBlock<P> p1;

#if defined(REPRO)
ParameterBlock<P> p2;
#endif

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    gOutput[tid] = gInput[tid] + p1.t.Load(int3(0,0,0)).x;
}
