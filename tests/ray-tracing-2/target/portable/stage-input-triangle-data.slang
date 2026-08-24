//TEST:SIMPLE(filecheck=HLSLCH): -experimental-feature -entry ClosestHit -stage closesthit -target hlsl
//TEST:SIMPLE(filecheck=HLSLAH): -experimental-feature -entry AnyHit -stage anyhit -target hlsl
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry ClosestHit -stage closesthit -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry ClosestHit -stage closesthit -dump-ir -no-codegen
//TEST:SIMPLE(filecheck=DXILCH): -experimental-feature -entry ClosestHit -stage closesthit -target dxil -profile sm_6_7
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

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

void applyClosestHitTriangleData(rt::ClosestHitInput<HitContext> input)
{
    let triangle = input.triangle;
    input.payload.value = triangle.barycentricCoord.x + (triangle.frontFacing ? 1.0 : 0.0);
}

struct ClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        applyClosestHitTriangleData(input);
    }
}

// HLSLCH-DAG: void applyClosestHitTriangleData_0(inout TestPayload_0 payload_0, float2 attributes_0)
// HLSLCH-DAG: HitKind()
// HLSLCH-DAG: struct StructuralTriangleHitAttributes_0
// HLSLCH-DAG: float2 barycentrics_0;
// HLSLCH-DAG: [shader("closesthit")]void ClosestHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload, StructuralTriangleHitAttributes_0 attributes_{{[0-9]+}} : SV_IntersectionAttributes)
// SPIRV-DAG: OpVariable %_ptr_IncomingRayPayloadKHR_TestPayload IncomingRayPayloadKHR
// SPIRV-DAG: OpVariable %_ptr_HitAttributeKHR_StructuralTriangleHitAttributes HitAttributeKHR
// IR: structuralRayTracingGetTriangleBarycentricCoord
// IR: structuralRayTracingGetTriangleFrontFacing
// DXILCH: define void
// DXILCH: !dx.dxrPayloadAnnotations

void applyAnyHitTriangleData(rt::AnyHitInput<HitContext> input)
{
    let triangle = input.triangle;
    input.payload.value = triangle.barycentricCoord.y + (triangle.frontFacing ? 2.0 : 0.0);
}

struct AnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        applyAnyHitTriangleData(input);
    }
}

// HLSLAH-DAG: void applyAnyHitTriangleData_0(inout TestPayload_0 payload_0, float2 attributes_0)
// HLSLAH-DAG: HitKind()
// HLSLAH-DAG: [shader("anyhit")]void AnyHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload, StructuralTriangleHitAttributes_0 attributes_{{[0-9]+}} : SV_IntersectionAttributes)
// DXILAH: define void
// DXILAH: !dx.dxrPayloadAnnotations
