// gh-841.slang

//TEST:CROSS_COMPILE: -profile ps_5_0 -entry main -target spirv-assembly

// GitHub issue #841: failing to emit `flat` modifier in output GLSL when required

struct RasterVertex
{
	float4 c : COLOR;
	uint u : FLAGS;
};

float4 main(RasterVertex v) : SV_Target
{
	float4 result = v.c;
	if((v.u & 1) != 0)
	    result += 1.0;
   return result;
}
