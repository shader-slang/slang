//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct Payload
{
    uint value;
}

struct Attributes
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
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
    typealias Record = void;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct CandidateIntersection : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(rt::IntersectionInput<Context> input)
    {
        // The generated Metal intersection function receives `values` from its IFT resource
        // bindings. This access verifies the Intersection half of the combined candidate adapter.
        Attributes attributes = { values[0] };
        input.reportHit(1.0, attributes);
    }
}

struct CandidateAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;

    void invoke(rt::AnyHitInput<Context> input)
    {
        // Metal combines the source Intersection and AnyHit stages in one native function. Both
        // source stages must see the same table-bound global resource.
        input.payload.value = input.attributes.value + values[1];
    }
}

struct CandidateClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.value += 100;
    }
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = 999;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = CandidateClosestHit;
    typealias AnyHit = CandidateAnyHit;
    typealias Intersection = CandidateIntersection;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

// Preserve the binding order used by the Metal runtime host. `values` deliberately follows the
// existing scene, structural descriptor, and result bindings, so reflection assigns buffer 3.
rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<uint> results;
RWStructuredBuffer<uint> values;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.ray.origin = float3(0.0, 0.0, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 10.0;
    desc.instanceMask = 0xff;

    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    results[0] = payload.value;
}

// The single bounding-box adapter contains both the source Intersection and AnyHit behavior and
// receives their shared resource through the reflected Metal buffer binding.
// METAL-LABEL: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing){{\]\]}}
// METAL-SAME: uint device* values_{{[0-9]+}} {{\[\[}}buffer(3){{\]\]}})
// METAL: ->values_0 = values_{{[0-9]+}};
