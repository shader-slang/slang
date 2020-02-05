// rw-buffer.slang

// Confirm that writing into a `RWBuffer` generates appropriate GLSL/SPIR-V.

//TEST:CROSS_COMPILE: -profile ps_5_0 -entry main -target spirv-assembly

RWBuffer<float> buffer;


float4 main(float u : U, int idx : IDX) : SV_Target
{
	buffer[idx] = u;
	return u;
}
