// entry-point-params.slang

// Test that entry-point `uniform` parameters on ray-tracing
// shaders properly map to the "shader record" in SPIR-V output.

//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -profile glsl_460+spirv_1_4 -stage raygeneration -entry main -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -allow-glsl -stage raygeneration -entry main -target spirv-assembly -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName
{
    uint data[];
} outputBuffer;

// CHECK_GLSL: void main(
// CHECK_SPIRV: OpEntryPoint
void main(
	uniform float value)
{
    outputBuffer.data[DispatchRaysIndex().x] = value;
}
