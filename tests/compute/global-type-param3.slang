//TEST(smoke,compute):COMPARE_COMPUTE:-xslang -use-ir
//TEST_INPUT: cbuffer(data=[1.0], stride=4):dxbinding(0),glbinding(0)
//TEST_INPUT: ubuffer(data=[0], stride=4):dxbinding(0),glbinding(0),out
//TEST_INPUT: type Impl

RWStructuredBuffer<float> outputBuffer;

interface IBase
{
    float compute();
}

struct Impl : IBase
{
    float base;  // = 1.0
    float compute()
    {
        return 1.0;
    }
};

__generic_param TImpl : IBase;

ParameterBlock<TImpl> impl;

float doCompute<T:IBase>(T t)
{
    return t.compute();
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	float outVal = doCompute<TImpl>(impl); 
	outputBuffer[tid] = outVal;
}