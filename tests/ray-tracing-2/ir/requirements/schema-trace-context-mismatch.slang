//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl

import slang.raytracing;

struct Payload
{
    uint value;
}
struct Record {}

struct SchemaTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct EntryTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = EntryTraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = ::Record;
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = EntryTraceContext;
    typealias Payload = ::Payload;
    typealias Record = ::Record;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input) {}
}

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = EntryTraceContext;
    typealias CallableData = uint;
    typealias Record = ::Record;
}

struct Callable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<CallableContext> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = SchemaTraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::CallableShaderList<Callable>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<Schema> tracer;
    Payload payload = {};
    tracer.trace({}, scene, program, payload);
//CHECK:        ^ structural ray-tracing entry uses a different trace context
//CHECK:        ^ entry 'Callable' uses trace context 'EntryTraceContext', but schema 'Schema' requires 'SchemaTraceContext'
//CHECK:        ^ structural ray-tracing entry uses a different trace context
//CHECK:        ^ entry 'Miss' uses trace context 'EntryTraceContext', but schema 'Schema' requires 'SchemaTraceContext'
//CHECK:        ^ structural ray-tracing entry uses a different trace context
//CHECK:        ^ entry 'HitGroup' uses trace context 'EntryTraceContext', but schema 'Schema' requires 'SchemaTraceContext'
}
