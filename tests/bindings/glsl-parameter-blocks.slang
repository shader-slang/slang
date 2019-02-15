//TEST:CROSS_COMPILE: -profile ps_5_0 -entry main -target spirv-assembly

struct Test
{
    float4 a;
	Texture2D t;
	SamplerState s;
};

ParameterBlock<Test> gTest;

float4 main(float2 uv : UV)
{
	return gTest.a + gTest.t.Sample(gTest.s, uv);
}
