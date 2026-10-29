//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -entry main -stage compute -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -entry main -stage compute -target metal -no-codegen

import slang.raytracing.portable;

struct TestPayload
{
    uint value;
}

struct TestRecord
{}

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

T makeValue<T>()
{
    return T();
}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = makeValue<rt::ClosestHitInput<HitContext>>().payload.value;
//CHECK:        ^^^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
//CHECK:        ^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' cannot be used as a generic type argument
}
