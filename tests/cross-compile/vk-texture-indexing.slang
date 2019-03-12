// vk-texture-indexing.slang

//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage fragment

struct Params
{
    Texture2D<float> textures[10];
    SamplerState sampler;
};

ParameterBlock<Params> gParams;

float fetchData(uint2 coords, uint index)
{
    return gParams.textures[NonUniformResourceIndex(index)][coords];
}

float4 main(
	uint3 uv : UV)
    : SV_Target
{
    float v = fetchData(uv.xy, uv.z);
    return v;
}
