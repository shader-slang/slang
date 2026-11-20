//TEST:COMPARE_COMPUTE(targets=all):

// Test that `break` from a loop works.

int test(int inVal)
{
	int ii = 0;
	for(;;)
	{
		if(ii >= inVal)
			break;
	    ii++;
	}
	return -ii;
}

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
	int inVal = outputBuffer[tid];
	int outVal = test(inVal);
	outputBuffer[tid] = outVal;
}