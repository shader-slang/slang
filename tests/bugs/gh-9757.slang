// Test for issue #9757: InternalError when HitObject::TraceRay payload is an empty struct.
// The empty logical payload is erased, but the native dispatch still needs a separate physical
// payload variable. Check its type, storage class, and use rather than padding the source Payload.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -capability spvShaderInvocationReorderNV -entry main -stage raygeneration -validate-ir

struct Payload{};

RaytracingAccelerationStructure tlas;
RWTexture2D<float> outImage;

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.0;
    ray.TMax = 1e30;

    Payload payload;
    HitObject hitObj = HitObject::TraceRay(tlas, 0, 0xFF, 0, 0, 0, ray, payload);
    if (hitObj.IsHit())
    {
        outImage[DispatchRaysIndex().xy] = 1.0f;
    }
}

// CHECK: OpMemberName %[[payloadType:EmptyRayPayload[A-Za-z0-9_]*]] 0 "_slang_dummy"
// CHECK: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// CHECK: %[[payloadType]] = OpTypeStruct %[[uint]]{{$}}
// CHECK: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer RayPayloadKHR %[[payloadType]]
// CHECK: %[[payload:[A-Za-z0-9_]+]] = OpVariable %[[pointer]] RayPayloadKHR
// CHECK: OpHitObjectTraceRayNV {{.*}} %[[payload]]{{$}}
