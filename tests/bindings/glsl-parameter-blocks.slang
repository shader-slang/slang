#version 450 core
//TEST:CROSS_COMPILE: -profile ps_5_0 -entry main -target spirv-assembly

// Note: disabled because the translation of `Texture2D.Sample()`
// requires handling of local variables with resource types in the IR.

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
