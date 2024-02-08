// closesthit.slang
//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -profile glsl_460+GL_EXT_ray_tracing -stage closesthit -entry main -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -allow-glsl -stage closesthit -entry main -target spirv-assembly -emit-spirv-directly

struct ReflectionRay
{
    float4 color;
};

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName
{
    vec4 data[];
} colorBuffer;

layout(shaderRecordNV)
cbuffer ShaderRecord
{
	uint shaderRecordID;
}

// CHECK_GLSL: void main(
// CHECK_SPIRV: OpEntryPoint
void main(
	BuiltInTriangleIntersectionAttributes 	attributes,
	in out ReflectionRay 					ioPayload)
{
	uint materialID = (InstanceIndex() << 1)
		+ InstanceID()
		+ PrimitiveIndex()
		+ HitKind()
		+ shaderRecordID;

    float4 color = colorBuffer.data[materialID];

	color *= RayTCurrent() - RayTMin();

	ioPayload.color = color;

	uint index = 0U;
	HitTriangleVertexPosition(index);

	index = 1U;
	HitTriangleVertexPosition(index);

	index = 2U;
	HitTriangleVertexPosition(index);
}
