// entry-point-params.slang

// Test that entry-point `uniform` parameters on ray-tracing
// shaders properly map to the "shader record" in SPIR-V output.

//TEST:CROSS_COMPILE: -profile glsl_460+spirv_1_4 -stage raygeneration -entry main -target spirv-assembly

RWStructuredBuffer<float> buffer;

void main(
	uniform float value) 
{
	buffer[DispatchRaysIndex().x] = value;
}
