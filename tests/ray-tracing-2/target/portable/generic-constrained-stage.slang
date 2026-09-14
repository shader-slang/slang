//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly

// A reusable stage may constrain a generic hit context to one primitive kind. The concrete
// specialization appears below in HitGroup's module-scope associated type, which exercises the
// source pattern that previously failed in shader-slang/slang#12822. The constraint also makes the
// triangle-only input property available inside the generic invoke method.

import slang.raytracing;

struct Payload
{
    float value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TriangleHitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct TriangleAnyHit<C> : rt::IAnyHitShader
    where C : rt::IHitContext
    where C.Primitive == rt::TrianglePrimitive
{
    typealias Context = C;

    void invoke(rt::AnyHitInput<Context> input)
    {
        if (input.triangle.barycentricCoord.x < 0.0)
            input.ignoreHit();
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = TriangleHitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = TriangleAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// HLSL-DAG: [shader("raygeneration")]void main()
// HLSL-DAG: [shader("anyhit")]void TriangleAnyHit_Sh{{[0-9A-Fa-f]+}}(
// HLSL-DAG: float2 barycentrics_0;
// HLSL-DAG: IgnoreHit();
// SPIRV-DAG: OpEntryPoint RayGenerationKHR %main "main"
// SPIRV-DAG: OpEntryPoint AnyHitKHR {{.*}} "TriangleAnyHit_Sh{{[0-9A-Fa-f]+}}"
// SPIRV-DAG: OpVariable {{.*}} HitAttributeKHR
