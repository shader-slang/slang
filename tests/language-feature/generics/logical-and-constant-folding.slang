//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=gInput
RWStructuredBuffer<float> gInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<float> gOutput;

//CHK:1
//CHK-NEXT:2
//CHK-NEXT:3
//CHK-NEXT:4

struct AppInput<bool COND0, int COND1>
{
    float3 position;
    Conditional<float4, COND0 && (0 < COND1)> attr;
};

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    AppInput<true, 1> t1;
    t1.attr.set(gInput[tid]);

    if (let value = t1.attr.get())
        gOutput[tid] = value.x;
}
