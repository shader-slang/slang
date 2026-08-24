//TEST:SIMPLE(filecheck=HLSLCH): -experimental-feature -entry ClosestHit -stage closesthit -target hlsl
//TEST:SIMPLE(filecheck=HLSLAH): -experimental-feature -entry AnyHit -stage anyhit -target hlsl
//TEST:SIMPLE(filecheck=HLSLMS): -experimental-feature -entry Miss -stage miss -target hlsl
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry ClosestHit -stage closesthit -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry ClosestHit -stage closesthit -dump-ir -no-codegen
//TEST:SIMPLE(filecheck=DXIL): -experimental-feature -entry Miss -stage miss -target dxil -profile sm_6_7

import slang.raytracing;

struct TestPayload
{
    uint value;
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

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias Record = TestRecord;
}

RWStructuredBuffer<uint> output;

void updateClosestHitPayload(rt::ClosestHitInput<HitContext> input)
{
    input.payload.value += 1;
}

struct ClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        updateClosestHitPayload(input);
        output[0] = input.payload.value;
    }
}

// HLSLCH: void updateClosestHitPayload_0(inout TestPayload_0 payload_0)
// HLSLCH: [shader("closesthit")]void ClosestHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload)
// SPIRV: OpVariable %_ptr_IncomingRayPayloadKHR_TestPayload IncomingRayPayloadKHR
// IR: structuralRayTracingGetPayload

void updateAnyHitPayload(rt::AnyHitInput<HitContext> input)
{
    input.payload.value += 2;
}

struct AnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        updateAnyHitPayload(input);
    }
}

// HLSLAH: void updateAnyHitPayload_0(inout TestPayload_0 payload_0)
// HLSLAH: [shader("anyhit")]void AnyHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload)

void updateMissPayload(rt::MissInput<MissContext> input)
{
    input.payload.value += 3;
}

struct Miss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        updateMissPayload(input);
    }
}

// HLSLMS: void updateMissPayload_0(inout TestPayload_0 payload_0)
// HLSLMS: [shader("miss")]void Miss(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload)
// DXIL: define void
// DXIL: !dx.dxrPayloadAnnotations
