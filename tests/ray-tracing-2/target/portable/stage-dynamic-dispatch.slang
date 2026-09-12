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

struct HitAttributes
{
    float value;
}

struct ProceduralHitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<HitAttributes>;
    typealias Record = void;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        IShape shape = makeShape(input.payload.value);
        input.payload.value = shape.area();
    }
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        IShape shape = makeShape(input.payload.value);
        input.payload.value = shape.area();
    }
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = ProceduralHitContext;
    void invoke(rt::IntersectionInput<ProceduralHitContext> input)
    {
        IShape shape = makeShape(input.primitiveIndex);
        output[2] = shape.area();
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = ::ClosestHit;
    typealias AnyHit = ::AnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct ProceduralHitGroup : rt::IHitGroup
{
    typealias Context = ProceduralHitContext;
    typealias ClosestHit = rt::NoClosestHit<ProceduralHitContext>;
    typealias AnyHit = rt::NoAnyHit<ProceduralHitContext>;
    typealias Intersection = ::Intersection;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input)
    {
        IShape shape = makeShape(input.payload.value);
        input.payload.value = shape.area();
    }
}

struct CallableData
{
    uint value;
}

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = ::CallableData;
    typealias Record = void;
}

struct Callable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<CallableContext> input)
    {
        IShape shape = makeShape(input.data.value);
        input.data.value = shape.area();
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup, ProceduralHitGroup>;
    typealias MissShaders = rt::MissShaderList<::Miss>;
    typealias CallableShaders = rt::CallableShaderList<::Callable>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
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
