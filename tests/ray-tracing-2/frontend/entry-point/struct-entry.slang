//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry ClosestHit -stage closesthit -target hlsl
//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry ClosestHit -target hlsl

// CHECK: RWStructuredBuffer<uint > outputBuffer_0
// CHECK: [shader("closesthit")]void ClosestHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload, StructuralTriangleHitAttributes_0 attributes_{{[0-9]+}} : SV_IntersectionAttributes)
// CHECK: ClosestHit_invoke_0();

import slang.raytracing;

RWStructuredBuffer<uint> outputBuffer;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    static const uint result = 0x12345678;

    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        outputBuffer[0] = result;
    }
}
