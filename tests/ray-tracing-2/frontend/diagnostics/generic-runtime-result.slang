//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -entry main -stage compute -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -entry main -stage compute -target metal

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestRecord
{}

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

T makeValue<T>()
{
    return T();
}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = makeValue<rt::ClosestHitInput<HitContext>>().payload.value;
//CHECK:                                                  ^ storage of a structural ray-tracing stage input
//CHECK:                                                  ^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter
}
