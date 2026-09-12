//TEST:SIMPLE(filecheck=FORWARD): -experimental-feature -entry main -stage raygeneration -target metal -DREVERSE=0
//TEST:SIMPLE(filecheck=REVERSE): -experimental-feature -entry main -stage raygeneration -target metal -DREVERSE=1

import slang.raytracing;

struct PayloadA
{
    uint value;
}

struct PayloadB
{
    float value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContextA : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadA;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct HitContextB : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadB;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct AnyHitA : rt::IAnyHitShader
{
    typealias Context = HitContextA;

    void invoke(rt::AnyHitInput<Context> input)
    {
        input.payload.value = input.triangle.frontFacing ? 1 : 0;
    }
}

struct AnyHitB : rt::IAnyHitShader
{
    typealias Context = HitContextB;

    void invoke(rt::AnyHitInput<Context> input)
    {
        input.payload.value = 2.0;
    }
}

struct HitGroupA : rt::IHitGroup
{
    typealias Context = HitContextA;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = AnyHitA;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct HitGroupB : rt::IHitGroup
{
    typealias Context = HitContextB;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = AnyHitB;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroupA, HitGroupB>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<float2> output;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    rt::RayTracer<Schema> tracer;
    PayloadA a = {};
    PayloadB b = {};
#if REVERSE
    tracer.trace(desc, scene, program, b);
    tracer.trace(desc, scene, program, a);
#else
    tracer.trace(desc, scene, program, a);
    tracer.trace(desc, scene, program, b);
#endif
    output[0] = float2(a.value, b.value);
}

// Both compilation orders must produce the same schema-declared partition order and the same
// normalized table signatures. In particular, the first trace visited must not freeze the other
// payload's descriptor or tag requirements.
// FORWARD-DAG: struct Schema_payload0_rayData_0
// FORWARD-DAG: struct Schema_payload1_rayData_0
// FORWARD-DAG: intersection_function_table<metal::raytracing::instancing, metal::raytracing::triangle_data> intersectionFunctions0_0;
// FORWARD-DAG: intersection_function_table<metal::raytracing::instancing> intersectionFunctions1_0;
// REVERSE-DAG: struct Schema_payload0_rayData_0
// REVERSE-DAG: struct Schema_payload1_rayData_0
// REVERSE-DAG: intersection_function_table<metal::raytracing::instancing, metal::raytracing::triangle_data> intersectionFunctions0_0;
// REVERSE-DAG: intersection_function_table<metal::raytracing::instancing> intersectionFunctions1_0;
