//TEST:COMPARE_HLSL:-no-mangle -profile cs_5_0

// Check output for `[allow_uav_conditional]`

#ifndef __SLANG__
#define gBuffer gBuffer_0
#endif

RWStructuredBuffer<uint> gBuffer : register(u0);

[numthreads(16,1,1)]
void main(
	uint tid : SV_DispatchThreadID)
{
	uint index = tid;

	[allow_uav_condition]
	for(;;)
	{
		if(gBuffer[index] == 0)
			break;
		index = gBuffer[index];
		gBuffer[index]--;
	}
}
