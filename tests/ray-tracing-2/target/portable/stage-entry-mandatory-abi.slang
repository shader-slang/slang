//TEST:SIMPLE(filecheck=HLSLCUSTOM): -experimental-feature -entry EmptyCustomClosestHit -stage closesthit -target hlsl
//TEST:SIMPLE(filecheck=HLSLTRIANGLE): -experimental-feature -entry EmptyTriangleAnyHit -stage anyhit -target hlsl
//TEST:SIMPLE(filecheck=HLSLMISS): -experimental-feature -entry EmptyMiss -stage miss -target hlsl
//TEST:SIMPLE(filecheck=HLSLCALLABLE): -experimental-feature -entry EmptyCallable -stage callable -target hlsl
//TEST:SIMPLE(filecheck=HLSLCALLABLEUSED): -experimental-feature -entry UsingCallable -stage callable -target hlsl
//TEST:SIMPLE(filecheck=SPIRVCALLABLE): -experimental-feature -entry UsingCallable -stage callable -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=DXILCUSTOM): -experimental-feature -entry EmptyCustomClosestHit -stage closesthit -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=DXILTRIANGLE): -experimental-feature -entry EmptyTriangleAnyHit -stage anyhit -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=DXILMISS): -experimental-feature -entry EmptyMiss -stage miss -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=DXILCALLABLE): -experimental-feature -entry EmptyCallable -stage callable -target dxil -profile sm_6_7

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

struct TestCustomAttributes
{
    float2 value;
}

struct CustomHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<TestCustomAttributes>;
    typealias Record = TestRecord;
}

struct TriangleHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestMissContext : rt::IMissGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias Record = TestRecord;
}

struct TestCallableData
{
    uint value;
}

struct TestCallableContext : rt::ICallableGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = TestCallableData;
    typealias Record = TestRecord;
}

struct EmptyCustomClosestHit : rt::IClosestHitShader<CustomHitContext>
{
    void invoke(rt::ClosestHitInput<CustomHitContext> input) {}
}

struct EmptyTriangleAnyHit : rt::IAnyHitShader<TriangleHitContext>
{
    void invoke(rt::AnyHitInput<TriangleHitContext> input) {}
}

struct EmptyMiss : rt::IMissShader<TestMissContext>
{
    void invoke(rt::MissInput<TestMissContext> input) {}
}

struct EmptyCallable : rt::ICallableShader<TestCallableContext>
{
    void invoke(rt::CallableInput<TestCallableContext> input) {}
}

void updateCallableData(rt::CallableInput<TestCallableContext> input)
{
    input.data.value++;
}

struct UsingCallable : rt::ICallableShader<TestCallableContext>
{
    void invoke(rt::CallableInput<TestCallableContext> input)
    {
        updateCallableData(input);
    }
}

// HLSLCUSTOM: [shader("closesthit")]void EmptyCustomClosestHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload, TestCustomAttributes_0 attributes_{{[0-9]+}} : SV_IntersectionAttributes)

// HLSLTRIANGLE: struct StructuralTriangleHitAttributes_0
// HLSLTRIANGLE: [shader("anyhit")]void EmptyTriangleAnyHit(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload, StructuralTriangleHitAttributes_0 attributes_{{[0-9]+}} : SV_IntersectionAttributes)

// HLSLMISS: [shader("miss")]void EmptyMiss(inout TestPayload_0 payload_{{[0-9]+}} : SV_RayPayload)

// HLSLCALLABLE: [shader("callable")]void EmptyCallable(inout TestCallableData_0 data_{{[0-9]+}})

// HLSLCALLABLEUSED: void updateCallableData_0(inout TestCallableData_0 data_{{[0-9]+}})
// HLSLCALLABLEUSED: [shader("callable")]void UsingCallable(inout TestCallableData_0 data_{{[0-9]+}})

// SPIRVCALLABLE: OpEntryPoint CallableKHR %UsingCallable "main" %data
// SPIRVCALLABLE: %_ptr_IncomingCallableDataKHR_TestCallableData = OpTypePointer IncomingCallableDataKHR %TestCallableData
// SPIRVCALLABLE: %data = OpVariable %_ptr_IncomingCallableDataKHR_TestCallableData IncomingCallableDataKHR

// DXILCUSTOM: define void
// DXILCUSTOM: !dx.dxrPayloadAnnotations
// DXILTRIANGLE: define void
// DXILTRIANGLE: !dx.dxrPayloadAnnotations
// DXILMISS: define void
// DXILMISS: !dx.dxrPayloadAnnotations
// DXILCALLABLE: define void
