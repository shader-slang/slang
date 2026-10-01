//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly

import slang.raytracing;

// A specialization constant has a default value, but its final value is selected after Slang has
// fixed the native payload ABI. It therefore cannot determine a structural payload's array size.
[vk::constant_id(0)]
const int payloadElementCount = 4;

struct SymbolicSizePayload
{
    float values[payloadElementCount];
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = SymbolicSizePayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct TestHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestHitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<TestSchema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<TestSchema> tracer;
    SymbolicSizePayload payload = {};
    tracer.trace({}, scene, program, payload);
//CHECK:        ^ structural ray-tracing ABI data is not portable plain data
//CHECK:        ^ type 'SymbolicSizePayload' is used as structural ray-tracing payload data but does not have a fixed-size, copyable plain-data representation
}
