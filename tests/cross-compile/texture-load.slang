// texture-load.slang

// Confirm that texture `Load` operations yield the
// expected type when compiled to SPIR-V.

//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage compute

Texture2D<float2>   inputTexture;
RWTexture2D<float2> outputTexture;

cbuffer C
{
	int2 pos;
}

[numthreads(16, 16, 1)]
void main(uint2 pixelIndex : SV_DispatchThreadID)
{
	float2 tmp = inputTexture.Load(int3(pos,0));
    outputTexture[pos] = tmp;
}
