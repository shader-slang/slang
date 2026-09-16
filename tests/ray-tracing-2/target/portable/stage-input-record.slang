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
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<HitAttributes>;
    typealias Record = HitRecord;
}

struct MissRecord
{
    uint value;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
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

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = ::CallableData;
    typealias Record = CallableRecord;
}

RWStructuredBuffer<uint> output;

struct ExistingConstants
{
    uint value;
}

// Reserve a user resource in space0 so the generated record buffer must use its compiler-owned
// space instead of a hard-coded global binding. Every standalone stage reads this value to keep
// the user binding live in its independently emitted HLSL library.
ConstantBuffer<ExistingConstants> existingConstants : register(b0);

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        output[0] = input.record.value + existingConstants.value;
    }
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        output[1] = input.record.value + existingConstants.value;
    }
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;
    void invoke(rt::IntersectionInput<HitContext> input)
    {
        output[2] = input.record.value + existingConstants.value;
    }
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input)
    {
        output[3] = input.record.value + existingConstants.value;
    }
}

struct Callable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<CallableContext> input)
    {
        output[4] = input.record.value + input.data.value + existingConstants.value;
    }
}

// HLSLCH: uint value_0;
// HLSLCH: cbuffer record_0 : register(b0, space1)
// HLSLCH: output_0[int(0)] = record_{{[0-9]+}}.value_{{[0-9]+}} + existingConstants_0.value_{{[0-9]+}};
// HLSLAH: uint value_0;
// HLSLAH: cbuffer record_0 : register(b0, space1)
// HLSLAH: output_0[int(1)] = record_{{[0-9]+}}.value_{{[0-9]+}} + existingConstants_0.value_{{[0-9]+}};
// HLSLIS: uint value_0;
// HLSLIS: cbuffer record_0 : register(b0, space1)
// HLSLIS: output_0[int(2)] = record_{{[0-9]+}}.value_{{[0-9]+}} + existingConstants_0.value_{{[0-9]+}};
// HLSLMS: uint value_0;
// HLSLMS: cbuffer record_0 : register(b0, space1)
// HLSLMS: output_0[int(3)] = record_{{[0-9]+}}.value_{{[0-9]+}} + existingConstants_0.value_{{[0-9]+}};
// HLSLCB: uint value_0;
// HLSLCB: cbuffer record_0 : register(b0, space1)
// HLSLCB: output_0[int(4)] = record_{{[0-9]+}}.value_{{[0-9]+}} + data_{{[0-9]+}}.value_{{[0-9]+}} + existingConstants_0.value_{{[0-9]+}};

// DXILRECORD: ; cbuffer record_0
// DXILRECORD: ; record_0{{.*}}cbuffer{{.*}}cb0{{.*}}space1

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
