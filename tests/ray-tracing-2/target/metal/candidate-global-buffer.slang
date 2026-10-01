//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct RuntimePayload
{
    uint value;
}

struct RuntimeTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Payload = RuntimePayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

RWStructuredBuffer<uint> values;
RWStructuredBuffer<uint> rayGenerationOnly;

uint readCandidateValue()
{
    return values[0];
}

struct RuntimeAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        // A Metal intersection function cannot read a module-scope resource directly. The
        // compiler must preserve this buffer's reflected binding and add it to the generated
        // candidate signature, including when the use is reached through an ordinary helper.
        input.payload.value += readCandidateValue();
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = RuntimeAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = RuntimeTraceContext;
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
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    RuntimePayload payload = { rayGenerationOnly[0] };
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// Only `values` is reachable from the triangle candidate. `rayGenerationOnly`, `scene`, and
// `program` must not leak into its native signature. Closing the signature immediately after
// `values` checks that the lowering is demand-driven.
// METAL-LABEL: {{\[\[}}intersection(triangle, metal::raytracing::instancing){{\]\]}}
// METAL-NOT: rayGenerationOnly
// METAL-NOT: scene
// METAL-NOT: program_resources
// METAL-SAME: uint device* values_{{[0-9]+}} {{\[\[}}buffer(0){{\]\]}})
// METAL: ->values_0 = values_{{[0-9]+}};

// The schema has no bounding-box candidate logic, so its reject-all table entry needs no buffer.
// METAL-LABEL: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing){{\]\]}}
// METAL-SAME: ray_data Schema_rayData_0& rayData_{{[0-9]+}} {{\[\[}}payload{{\]\]}})
