// closesthit.slang
//TEST:CROSS_COMPILE: -profile sm_6_3 -stage closesthit -entry main -target spirv-assembly

struct ReflectionRay
{
    float4 color;
};

StructuredBuffer<float4> colors;

void main(
	BuiltInTriangleIntersectionAttributes 	attributes,
	in out ReflectionRay 					ioPayload)
{
	uint materialID = InstanceIndex()
		+ InstanceID()
		+ PrimitiveIndex()
		+ HitKind();

	float4 color = colors[materialID];

	color *= RayTCurrent() - RayTMin();

	ioPayload.color = color;
}
