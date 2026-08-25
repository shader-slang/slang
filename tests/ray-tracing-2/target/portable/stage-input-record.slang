//TEST:SIMPLE(filecheck=HLSLCH): -experimental-feature -entry ClosestHit -stage closesthit -target hlsl
//TEST:SIMPLE(filecheck=HLSLAH): -experimental-feature -entry AnyHit -stage anyhit -target hlsl
//TEST:SIMPLE(filecheck=HLSLIS): -experimental-feature -entry Intersection -stage intersection -target hlsl
//TEST:SIMPLE(filecheck=HLSLMS): -experimental-feature -entry Miss -stage miss -target hlsl
//TEST:SIMPLE(filecheck=HLSLCB): -experimental-feature -entry Callable -stage callable -target hlsl
//TEST:SIMPLE(filecheck=DXILRECORD): -experimental-feature -entry ClosestHit -stage closesthit -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=DXILRECORD): -experimental-feature -entry AnyHit -stage anyhit -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=DXILRECORD): -experimental-feature -entry Intersection -stage intersection -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=DXILRECORD): -experimental-feature -entry Miss -stage miss -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=DXILRECORD): -experimental-feature -entry Callable -stage callable -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=SPVCH): -experimental-feature -entry ClosestHit -stage closesthit -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVAH): -experimental-feature -entry AnyHit -stage anyhit -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVIS): -experimental-feature -entry Intersection -stage intersection -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVMS): -experimental-feature -entry Miss -stage miss -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVCB): -experimental-feature -entry Callable -stage callable -target spirv-assembly -emit-spirv-directly

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitRecord
{
    uint value;
}

struct HitAttributes
{
    float value;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<HitAttributes>;
    typealias Record = HitRecord;
}

struct MissRecord
{
    uint value;
}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias Record = MissRecord;
}

struct CallableData
{
    uint value;
}

struct CallableRecord
{
    uint value;
}

struct CallableContext : rt::ICallableGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = ::CallableData;
    typealias Record = CallableRecord;
}

RWStructuredBuffer<uint> output;

struct ClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        output[0] = input.record.value;
    }
}

struct AnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        output[1] = input.record.value;
    }
}

struct Intersection : rt::IIntersectionShader<HitContext>
{
    void invoke(rt::IntersectionInput<HitContext> input)
    {
        output[2] = input.record.value;
    }
}

struct Miss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        output[3] = input.record.value;
    }
}

struct Callable : rt::ICallableShader<CallableContext>
{
    void invoke(rt::CallableInput<CallableContext> input)
    {
        output[4] = input.record.value + input.data.value;
    }
}

// HLSLCH: uint value_0;
// HLSLCH: cbuffer record_0
// HLSLCH: output_0[int(0)] = record_{{[0-9]+}}.value_0;
// HLSLAH: uint value_0;
// HLSLAH: cbuffer record_0
// HLSLAH: output_0[int(1)] = record_{{[0-9]+}}.value_0;
// HLSLIS: uint value_0;
// HLSLIS: cbuffer record_0
// HLSLIS: output_0[int(2)] = record_{{[0-9]+}}.value_0;
// HLSLMS: uint value_0;
// HLSLMS: cbuffer record_0
// HLSLMS: output_0[int(3)] = record_{{[0-9]+}}.value_0;
// HLSLCB: uint value_0;
// HLSLCB: cbuffer record_0
// HLSLCB: output_0[int(4)] = record_{{[0-9]+}}.value_0 + data_{{[0-9]+}}.value_1;

// DXILRECORD: cbuffer record_0

// SPVCH: OpEntryPoint ClosestHitKHR
// SPVCH: ShaderRecordBufferKHR
// SPVAH: OpEntryPoint AnyHitKHR
// SPVAH: ShaderRecordBufferKHR
// SPVIS: OpEntryPoint IntersectionKHR
// SPVIS: ShaderRecordBufferKHR
// SPVMS: OpEntryPoint MissKHR
// SPVMS: ShaderRecordBufferKHR
// SPVCB: OpEntryPoint CallableKHR
// SPVCB: ShaderRecordBufferKHR
