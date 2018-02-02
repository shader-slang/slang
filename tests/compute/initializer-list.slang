//TEST(compute):COMPARE_COMPUTE:

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out

struct Test
{
	float4 	a;
	uint 	b;
	float 	c;	
};


uint test(uint val)
{
	Test t = { float4(1.0f), 16, 99.0f };
	return val + t.b;
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	uint inVal = tid;
	uint outVal = test(inVal);

	outputBuffer[tid] = outVal;
}