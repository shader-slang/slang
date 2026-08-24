//TEST:SIMPLE(filecheck=HLSLCH): -experimental-feature -entry ClosestHit -stage closesthit -target hlsl
//TEST:SIMPLE(filecheck=HLSLAH): -experimental-feature -entry AnyHit -stage anyhit -target hlsl
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry ClosestHit -stage closesthit -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry ClosestHit -stage closesthit -dump-ir -no-codegen
//TEST:SIMPLE(filecheck=DXIL): -experimental-feature -entry ClosestHit -stage closesthit -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=DXILAH): -experimental-feature -entry AnyHit -stage anyhit -target dxil -profile sm_6_7

import slang.raytracing;

struct TestPayload
{
    float value;
}

struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct CustomAttributes
{
    float2 value;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<CustomAttributes>;
    typealias Record = TestRecord;
}

void applyClosestHitAttributes(rt::ClosestHitInput<HitContext> input)
{
    input.payload.value = input.attributes.value.x;
}

struct ClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        applyClosestHitAttributes(input);
    }
}

// HLSLCH: void applyClosestHitAttributes_0(inout TestPayload_0 payload_0, CustomAttributes_0 attributes_0)
// HLSLCH: [shader("closesthit")]void ClosestHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload, CustomAttributes_0 attributes_{{[0-9]+}} : SV_IntersectionAttributes)
// SPIRV-DAG: OpVariable %_ptr_IncomingRayPayloadKHR_TestPayload IncomingRayPayloadKHR
// SPIRV-DAG: OpVariable %_ptr_HitAttributeKHR_CustomAttributes HitAttributeKHR
// IR: structuralRayTracingGetHitAttributes
// DXIL: define void
// DXIL: !dx.dxrPayloadAnnotations

void applyAnyHitAttributes(rt::AnyHitInput<HitContext> input)
{
    input.payload.value = input.attributes.value.y;
}

struct AnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        applyAnyHitAttributes(input);
    }
}

// HLSLAH: void applyAnyHitAttributes_0(inout TestPayload_0 payload_0, CustomAttributes_0 attributes_0)
// HLSLAH: [shader("anyhit")]void AnyHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload, CustomAttributes_0 attributes_{{[0-9]+}} : SV_IntersectionAttributes)
// DXILAH: define void
// DXILAH: !dx.dxrPayloadAnnotations
