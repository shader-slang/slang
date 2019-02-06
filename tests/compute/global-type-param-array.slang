//TEST(compute):COMPARE_COMPUTE:
//TEST_INPUT: cbuffer(data=[1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0], stride=4):dxbinding(0),glbinding(0)
//TEST_INPUT: ubuffer(data=[0], stride=4):dxbinding(0),glbinding(0),out
//TEST_INPUT: type Pair<Arr<Base,1>, Pair<Arr<Base,2> , Base> >

RWStructuredBuffer<float> outputBuffer;
import globalTypeParamArrayShared;

float doCompute<T:IBase>(T t)
{
    return t.compute(1.0);
}

[numthreads(1, 1, 1)]
void computeMain<
	TImpl : IBase>(
	uniform ParameterBlock<TImpl> impl,
	uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	float outVal = doCompute<TImpl>(impl); 
	outputBuffer[tid] = outVal;
}