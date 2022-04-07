// closesthit.slang
//TEST:CROSS_COMPILE: -profile glsl_460+GL_NV_ray_tracing -stage closesthit -entry main -target spirv-assembly

struct ReflectionRay
{
    float4 color;
};

StructuredBuffer<float4> colors;

layout(shaderRecordNV)
cbuffer ShaderRecord
{
	uint shaderRecordID;
}

void main(
	BuiltInTriangleIntersectionAttributes 	attributes,
	in out ReflectionRay 					ioPayload)
{
	uint materialID = (InstanceIndex() << 1)
		+ InstanceID()
		+ PrimitiveIndex()
		+ HitKind()
		+ shaderRecordID;

	float4 color = colors[materialID];

	color *= RayTCurrent() - RayTMin();

	ioPayload.color = color;
}
