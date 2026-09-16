//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry FirstMiss -stage miss -entry SecondMiss -stage miss -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=DXIL): -experimental-feature -entry FirstMiss -stage miss -entry SecondMiss -stage miss -target dxil -profile sm_6_5

import slang.raytracing;

struct Payload
{
    uint value;
}

struct FirstRecord
{
    uint first;
}

struct SecondRecord
{
    uint2 second;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct FirstContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = ::FirstRecord;
}

struct SecondContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = ::SecondRecord;
}

struct FirstMiss : rt::IMissShader
{
    typealias Context = FirstContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = input.record.first;
    }
}

struct SecondMiss : rt::IMissShader
{
    typealias Context = SecondContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = input.record.second.x;
    }
}

// The component selects both stages into one target library. Their EntryPointLayouts allocate b0
// and b1 in the same compiler-owned space, and lowering must preserve that per-entry association.
// Checking the element type with each register prevents a hash-set iteration from silently
// swapping the reflected stage-to-binding mapping while still emitting the same register set.
// HLSL: cbuffer record_0 : register(b0)
// HLSL: FirstRecord_0 record_0;
// HLSL: cbuffer record_0 : register(b1)
// HLSL: SecondRecord_0 record_0;

// DXIL-DAG: ; record_{{[0-9]+}}{{.*}}cbuffer{{.*}}cb0
// DXIL-DAG: ; record_{{[0-9]+}}{{.*}}cbuffer{{.*}}cb1
