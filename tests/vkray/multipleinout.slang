//TEST:SIMPLE(filecheck=CHECK): -stage closesthit -entry main -target spirv -emit-spirv-directly

// This test checks whether the spirv generated when there are multiple inout or out variables, they 
// all get consolidated into one IncomingRayPayloadKHR.

struct ReflectionRay
{
    float4 color;
};

StructuredBuffer<float4> colors;

[shader("closesthit")]
void main(
    BuiltInTriangleIntersectionAttributes attributes,
    inout ReflectionRay ioPayload,
    out float3 dummy)
{
    uint materialID = (InstanceIndex() << 1)
        + InstanceID()
        + PrimitiveIndex()
        + HitKind();

    ioPayload.color = colors[materialID];
    dummy = HitTriangleVertexPosition(0);
}

// CHECK: OpCapability RayTracingKHR
// CHECK: OpCapability RayTracingPositionFetchKHR
// CHECK: OpCapability Shader
// CHECK: OpExtension "SPV_KHR_ray_tracing"
// CHECK: OpExtension "SPV_KHR_storage_buffer_storage_class"
// CHECK: OpExtension "SPV_KHR_ray_tracing_position_fetch"
// CHECK: OpMemoryModel Logical GLSL450
// CHECK: OpEntryPoint ClosestHitKHR %main "main" %{{.*}} %{{.*}} %gl_PrimitiveID %{{.*}} %gl_InstanceID %colors %{{.*}}

// CHECK: OpName %ReflectionRay "ReflectionRay"
// CHECK: OpMemberName %ReflectionRay 0 "color"
// CHECK: OpName %materialID "materialID"
// CHECK: OpName %StructuredBuffer "StructuredBuffer"
// CHECK: OpName %colors "colors"
// CHECK: OpName %main "main"

// CHECK-DAG: OpDecorate %gl_InstanceID BuiltIn InstanceId
// CHECK-DAG: OpDecorate %{{.*}} BuiltIn InstanceCustomIndexKHR
// CHECK-DAG: OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId
// CHECK-DAG: OpDecorate %{{.*}} BuiltIn HitKindKHR
// CHECK-DAG: OpDecorate %_runtimearr_v4float ArrayStride 16
// CHECK-DAG: OpDecorate %StructuredBuffer Block
// CHECK-DAG: OpDecorate %colors Binding 0
// CHECK-DAG: OpDecorate %colors DescriptorSet 0
// CHECK-DAG: OpDecorate %colors NonWritable
// CHECK-DAG: OpDecorate %{{.*}} BuiltIn HitTriangleVertexPositionsKHR

// CHECK: %ReflectionRay = OpTypeStruct %v4float
// CHECK: %_struct_{{.*}} = OpTypeStruct %ReflectionRay %v3float
// CHECK: %_ptr_IncomingRayPayloadKHR__struct_{{.*}} = OpTypePointer IncomingRayPayloadKHR %_struct_{{.*}}
// CHECK: %_ptr_IncomingRayPayloadKHR_ReflectionRay = OpTypePointer IncomingRayPayloadKHR %ReflectionRay
// CHECK: %_ptr_IncomingRayPayloadKHR_v3float = OpTypePointer IncomingRayPayloadKHR %v3float
// CHECK: %StructuredBuffer = OpTypeStruct %_runtimearr_v4float
// CHECK: %_ptr_StorageBuffer_StructuredBuffer = OpTypePointer StorageBuffer %StructuredBuffer
// CHECK: %_ptr_Input__arr_v3float_{{.*}} = OpTypePointer Input %_arr_v3float_{{.*}}

// CHECK: %main = OpFunction %void None %{{.*}}
// CHECK: %materialID = OpIAdd %uint %{{.*}} %{{.*}}
// CHECK: %{{.*}} = OpAccessChain %_ptr_StorageBuffer_v4float %colors %int_0 %materialID
// CHECK: %{{.*}} = OpLoad %v4float %{{.*}}
// CHECK: %{{.*}} = OpAccessChain %_ptr_Input_v3float %{{.*}} %uint_0
// CHECK: %{{.*}} = OpLoad %v3float %{{.*}}
