//TEST(compute):COMPARE_COMPUTE:
//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7], stride=4):out

// Test that `swith` statement works

int test(int inVal)
{
	int result = inVal;
	switch(inVal)
	{
	case 0:
		result = 16;
		break;

	case 2:
	case 5:
		result = inVal + 16;
		break;

	case 4:
		{}
		break;		

	default:
	case 3:
		result = 0;
		break;
	}

	return result;
}

RWStructuredBuffer<int> outputBuffer : register(u0);

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	int inVal = outputBuffer[tid];
	int outVal = test(inVal);
	outputBuffer[tid] = outVal;
}