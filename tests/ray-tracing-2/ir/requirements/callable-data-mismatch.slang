//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Record
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

struct ContextA : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = DataA;
    typealias Record = ::Record;
}

struct ContextB : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = DataB;
    typealias Record = ::Record;
}

struct CallableA : rt::ICallableShader
{
    typealias Context = ContextA;
    void invoke(rt::CallableInput<ContextA> input) {}
}

struct CallableB : rt::ICallableShader
{
    typealias Context = ContextB;
    void invoke(rt::CallableInput<ContextB> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::CallableShaderList<CallableA, CallableB>;
}

rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<Schema> tracer;
    DataA data = {};
    tracer.callShader<ContextA>(0, program, data);
//CHECK:                       ^ incompatible structural ray-tracing callable data
//CHECK:                       ^ callable shader 'CallableB' uses data type 'DataB', which does not match the required callable data type 'DataA'
}
