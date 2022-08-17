// image-load.slang

// This test confirms that `.Load()` on a `RWTexure*`
// gets properly converted to a call to `imageLoad`
// and not just `texelFetch` as it would for a `Texture*`.

//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage compute

struct Params
{
	RWTexture2DArray<float> tex;
}

ParameterBlock<Params> gParams;

void main(uint3 tid : SV_DispatchThreadID)
{
	float f = gParams.tex.Load(int3(int2(tid.xy), int(tid.z)));
}
