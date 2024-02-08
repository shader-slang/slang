//TEST:SIMPLE(filecheck=CHECK_GLSL): -profile glsl_460+GL_EXT_ray_tracing -stage closesthit -entry main -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -stage closesthit -entry main -target spirv-assembly -emit-spirv-directly


struct IntersectionPayload
{
    float3 triangleVerticeCommitted[3];
    float3 triangleVerticeCandidate[3];
};

RaytracingAccelerationStructure accelerationStructure;

// CHECK_GLSL: void main(
// CHECK_SPIRV: OpEntryPoint
void main(
	BuiltInTriangleIntersectionAttributes 	attributes,
	in out IntersectionPayload 				ioPayload)
{
	RayQuery<RAY_FLAG_NONE> rayQuery;

	uint instanceInclusionMask = 0x00;
	RayDesc rayDesc;
	rayQuery.TraceRayInline(accelerationStructure, RAY_FLAG_NONE, instanceInclusionMask, rayDesc);

	ioPayload.triangleVerticeCommitted = rayQuery.CandidateGetIntersectionTriangleVertexPositions();
	ioPayload.triangleVerticeCandidate = rayQuery.CommittedGetIntersectionTriangleVertexPositions();
}
