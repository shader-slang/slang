//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -shaderobj

struct Test
{
	float4 	a;
	uint 	b;
	float 	c;	
};


uint test(uint val)
{
    Test t = { { 1.0f, 1.0f, 1.0f, 1.0f }, 16, 99.0f };
	return val + t.b;
}

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	uint inVal = tid;
	uint outVal = test(inVal);

//BUF: 10
//BUF-NEXT: 11
//BUF-NEXT: 12
//BUF-NEXT: 13
	outputBuffer[tid] = outVal;
}