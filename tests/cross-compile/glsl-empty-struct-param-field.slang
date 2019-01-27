//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -profile ps_5_0

struct E
{
};

struct P
{
	E em;
	float4 param;
};
ParameterBlock<P> pblock;

float4 main(float4 pos : SV_POSITION)
{
	return pblock.param;
}