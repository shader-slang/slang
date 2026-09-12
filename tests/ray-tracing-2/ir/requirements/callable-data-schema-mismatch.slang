//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl

import slang.raytracing;

struct Payload
{
    uint value;
}

struct DataA
{
    uint value;
}

struct DataB
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct CallableContext<Data> : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = Data;
    typealias Record = void;
}

struct CallableA : rt::ICallableShader
{
    typealias Context = CallableContext<DataA>;
    void invoke(rt::CallableInput<Context> input) {}
}

struct CallableB : rt::ICallableShader
{
    typealias Context = CallableContext<DataB>;
    void invoke(rt::CallableInput<Context> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::CallableShaderList<CallableA, CallableB>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace({}, scene, program, payload);
//CHECK:        ^ incompatible structural ray-tracing callable data
//CHECK:        ^ callable shader 'CallableB' uses data type 'DataB', which does not match the required callable data type 'DataA'
}
