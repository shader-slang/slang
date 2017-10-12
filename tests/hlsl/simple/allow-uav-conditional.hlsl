//TEST:COMPARE_HLSL: -profile cs_5_0 -target dxbc-assembly

// Check output for `[allow_uav_conditional]`

RWStructuredBuffer<uint> gBuffer : register(u0);

[numthreads(16,1,1)]
void main(
	uint tid : SV_DispatchThreadID)
{
	uint index = tid;

	[allow_uav_condition]
	while(gBuffer[index] != 0)
	{
		index = gBuffer[index];
		gBuffer[index]--;
	}
}
