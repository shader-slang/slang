//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=BUF): -output-using-type
//TEST:COMPARE_COMPUTE(targets=metal, filecheck-buffer=BUF):

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static uint g_values[2] = { 0, 1 };

[numthreads(2, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;

	outputBuffer[tid] = g_values[tid];
    // BUF: 0
    // BUF: 1
}
