//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal -capability metallib_3_1

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

struct BoundingBoxContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
    typealias Record = void;
}

struct TriangleContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct CurveContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::CurvePrimitive;
    typealias Record = void;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct BoundingBoxClosestHit : rt::IClosestHitShader
{
    typealias Context = BoundingBoxContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.value = input.attributes.value;
    }
}

struct BoundingBoxIntersection : rt::IIntersectionShader
{
    typealias Context = BoundingBoxContext;

    void invoke(rt::IntersectionInput<Context> input)
    {
        Attributes attributes = { 7 };
        input.reportHit(1.0, attributes);
    }
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = 99;
    }
}

struct BoundingBoxGroup : rt::IHitGroup
{
    typealias Context = BoundingBoxContext;
    typealias ClosestHit = BoundingBoxClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = BoundingBoxIntersection;
}

// These two groups do not need custom candidate behavior. Once the bounding-box group activates
// the schema's IFT, the compiler still emits triangle and curve dispatchers at fixed indices zero
// and two. Their explicit accepting arms select these groups through the record function index,
// while both `NoClosestHit` entries reuse one reflected no-op visible function.
struct TriangleGroup : rt::IHitGroup
{
    typealias Context = TriangleContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct CurveGroup : rt::IHitGroup
{
    typealias Context = CurveContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<BoundingBoxGroup, TriangleGroup, CurveGroup>;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<uint> output;

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
    output[0] = payload.value;
}

// The three exported per-kind dispatchers occupy fixed IFT indices zero, one, and two. Unlike a
// native opaque entry, each built-in dispatcher consults the runtime record function index.
// METAL: intersection_function_table<metal::raytracing::instancing{{.*}}>
// The visible functions comprise one miss adapter, the real bounding-box closest-hit adapter, and
// one shared closest-hit placeholder installed at both placeholder function indices.
// METAL-COUNT-3: {{\[\[visible\]\]}} void __slang_structural_rt_
// METAL: {{\[\[}}intersection(triangle, metal::raytracing::instancing{{.*}})]]
// METAL: default:
// METAL: = { false, true };
// METAL: case int(1):
// METAL: = { true, true };
// METAL: case 4294967295U:
// METAL: = { true, true };
// METAL: {{\[\[}}intersection(curve, metal::raytracing::instancing{{.*}})]]
// METAL: default:
// METAL: = { false, true };
// METAL: case int(2):
// METAL: = { true, true };
// METAL: case 4294967295U:
// METAL: = { true, true };
// METAL: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing{{.*}})]]
// METAL: default:
// METAL: = { false, true, 0.0f };
// METAL: case int(0):
