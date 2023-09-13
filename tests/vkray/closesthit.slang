// closesthit.slang
//TEST:SIMPLE(filecheck=CHECK): -profile glsl_460+GL_NV_ray_tracing -stage closesthit -entry main -target spirv-assembly

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

// CHECK: OpCapability RayTracing
// CHECK: OpEntryPoint ClosestHitNV %main "main"
// CHECK: OpDecorate %gl_InstanceID BuiltIn InstanceId
// CHECK: OpDecorate %gl_InstanceCustomIndexNV BuiltIn InstanceCustomIndexNV
// CHECK: OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
// CHECK: OpDecorate %gl_HitKindNV BuiltIn HitKindNV
// CHECK: OpDecorate %gl_RayTmaxNV BuiltIn RayTmaxNV
// CHECK: OpDecorate %gl_RayTminNV BuiltIn RayTminNV
// CHECK: %ShaderRecord_0 = OpVariable %_ptr_ShaderRecordBufferNV__S{{.*}} ShaderRecordBufferNV
// CHECK: %{{.*}} = OpVariable %_ptr_IncomingRayPayloadNV_ReflectionRay_0 IncomingRayPayloadNV
// CHECK: %{{.*}} = OpLoad %int %gl_InstanceID
// CHECK: %{{.*}} = OpLoad %int %gl_InstanceCustomIndexNV
// CHECK: %{{.*}} = OpAccessChain %_ptr_ShaderRecordBufferNV_uint %ShaderRecord_0 %int_0
// CHECK: %{{.*}} = OpAccessChain %_ptr_IncomingRayPayloadNV_v4float %_S{{.*}} %int_0
