//TEST:SIMPLE(filecheck=LOCAL): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=EXTERNAL): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly -DUSE_EXTERNAL_CONFORMANCES=1 -conformance "Circle:IShape=0" -conformance "Square:IShape=1"

// A structural stage discovered from RayTracer.trace() must participate in
// type-flow specialization before its native entry-point adapter is synthesized.

import slang.raytracing;

interface IShape
{
    uint area();
}

struct Circle : IShape
{
    uint area() { return 3; }
}

struct Square : IShape
{
    uint area() { return 5; }
}

IShape makeShape(uint id)
{
#if USE_EXTERNAL_CONFORMANCES
    return createDynamicObject<IShape>(id);
#else
    if (id == 0)
        return Circle();
    return Square();
#endif
}

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

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct HitAttributes
{
    float value;
}

struct ProceduralHitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<HitAttributes>;
    typealias Record = void;
}

struct ClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        IShape shape = makeShape(input.payload.value);
        input.payload.value = shape.area();
    }
}

struct AnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        IShape shape = makeShape(input.payload.value);
        input.payload.value = shape.area();
    }
}

struct Intersection : rt::IIntersectionShader<ProceduralHitContext>
{
    void invoke(rt::IntersectionInput<ProceduralHitContext> input)
    {
        IShape shape = makeShape(input.primitiveIndex);
        output[2] = shape.area();
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = ::ClosestHit;
    typealias AnyHit = ::AnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct ProceduralHitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<1>;
    typealias Context = ProceduralHitContext;
    typealias ClosestHit = rt::NoClosestHit<ProceduralHitContext>;
    typealias AnyHit = rt::NoAnyHit<ProceduralHitContext>;
    typealias Intersection = ::Intersection;
}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = ::TraceContext;
    typealias Record = void;
}

struct Miss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        IShape shape = makeShape(input.payload.value);
        input.payload.value = shape.area();
    }
}

struct MissGroup : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = MissContext;
    typealias Miss = ::Miss;
}

struct CallableData
{
    uint value;
}

struct CallableContext : rt::ICallableGroupContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = ::CallableData;
    typealias Record = void;
}

struct Callable : rt::ICallableShader<CallableContext>
{
    void invoke(rt::CallableInput<CallableContext> input)
    {
        IShape shape = makeShape(input.data.value);
        input.data.value = shape.area();
    }
}

struct CallableGroup : rt::ICallableGroup
{
    typealias Slot = rt::CallableSlot<0>;
    typealias Context = CallableContext;
    typealias Callable = ::Callable;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<::TraceContext, HitGroup, ProceduralHitGroup>;
    typealias MissGroups = rt::MissGroupList<::TraceContext, MissGroup>;
    typealias CallableGroups = rt::CallableGroupList<::TraceContext, CallableGroup>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
    CallableData data = {payload.value};
    tracer.callShader<CallableContext>(0, program, data);
    output[5] = data.value;
}

// LOCAL-DAG: OpEntryPoint RayGenerationKHR %main "main"
// LOCAL-DAG: OpEntryPoint ClosestHitKHR %ClosestHit "ClosestHit"
// LOCAL-DAG: OpEntryPoint AnyHitKHR %AnyHit "AnyHit"
// LOCAL-DAG: OpEntryPoint IntersectionKHR %Intersection "Intersection"
// LOCAL-DAG: OpEntryPoint MissKHR %Miss "Miss"
// LOCAL-DAG: OpEntryPoint CallableKHR %Callable "Callable"
// LOCAL: OpSwitch

// EXTERNAL-DAG: OpEntryPoint RayGenerationKHR %main "main"
// EXTERNAL-DAG: OpEntryPoint ClosestHitKHR %ClosestHit "ClosestHit"
// EXTERNAL-DAG: OpEntryPoint AnyHitKHR %AnyHit "AnyHit"
// EXTERNAL-DAG: OpEntryPoint IntersectionKHR %Intersection "Intersection"
// EXTERNAL-DAG: OpEntryPoint MissKHR %Miss "Miss"
// EXTERNAL-DAG: OpEntryPoint CallableKHR %Callable "Callable"
// EXTERNAL: OpSwitch
