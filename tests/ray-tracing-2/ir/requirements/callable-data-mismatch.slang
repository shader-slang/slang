//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl

import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias Payload = ::Payload;
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

struct ContextA : rt::ICallableGroupContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = DataA;
    typealias Record = ::Record;
}

struct ContextB : rt::ICallableGroupContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = DataB;
    typealias Record = ::Record;
}

struct CallableA : rt::ICallableShader<ContextA>
{
    void invoke(rt::CallableInput<ContextA> input) {}
}

struct CallableB : rt::ICallableShader<ContextB>
{
    void invoke(rt::CallableInput<ContextB> input) {}
}

struct GroupA : rt::ICallableGroup
{
    typealias Slot = rt::CallableSlot<0>;
    typealias Context = ContextA;
    typealias Callable = CallableA;
}

struct GroupB : rt::ICallableGroup
{
    typealias Slot = rt::CallableSlot<1>;
    typealias Context = ContextB;
    typealias Callable = CallableB;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups<::TraceContext>;
    typealias MissGroups = rt::NoMissGroups<::TraceContext>;
    typealias CallableGroups = rt::CallableGroupList<::TraceContext, GroupA, GroupB>;
}

rt::TraceProgramDescriptor<ProgramLayout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<ProgramLayout> tracer;
    DataA data = {};
    tracer.callShader<ContextA>(0, program, data);
//CHECK:                       ^ incompatible structural ray-tracing callable data
//CHECK:                       ^ callable group slot 1 uses data type 'DataB', but this call expects 'DataA'
}
