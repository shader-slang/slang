//TEST:SIMPLE(filecheck=METAL,TRIE): -experimental-feature -entry main -stage raygeneration -target metal

// A max-levels traversal supplies the complete instance path to an intersection function. This
// test keeps candidate logic in both the fixed-function triangle path and the procedural AABB path
// so both generated dispatchers must accept Metal's array-valued instance attributes. Record
// selection traverses every instance index in that path; it must not reduce the path to one index.
import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<3>;
    typealias Motion = rt::NoMotion;
}

struct TriangleContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct TriangleAnyHit : rt::IAnyHitShader
{
    typealias Context = TriangleContext;
    void invoke(rt::AnyHitInput<TriangleContext> input) { input.payload.value += 1; }
}

struct TriangleGroup : rt::IHitGroup
{
    typealias Context = TriangleContext;
    typealias ClosestHit = rt::NoClosestHit<TriangleContext>;
    typealias AnyHit = TriangleAnyHit;
    typealias Intersection = rt::NoIntersection<TriangleContext>;
}

struct BoxAttributes
{
    uint value;
}

struct BoxContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<BoxAttributes>;
    typealias Record = void;
}

struct BoxIntersection : rt::IIntersectionShader
{
    typealias Context = BoxContext;
    void invoke(rt::IntersectionInput<BoxContext> input)
    {
        BoxAttributes attributes = {1};
        input.reportHit(1.0, attributes);
    }
}

struct BoxAnyHit : rt::IAnyHitShader
{
    typealias Context = BoxContext;
    void invoke(rt::AnyHitInput<BoxContext> input)
    {
        input.payload.value += input.attributes.value;
    }
}

struct BoxGroup : rt::IHitGroup
{
    typealias Context = BoxContext;
    typealias ClosestHit = rt::NoClosestHit<BoxContext>;
    typealias AnyHit = BoxAnyHit;
    typealias Intersection = BoxIntersection;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<TriangleGroup, BoxGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::MultiLevelAccelerationStructure<3> scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// METAL-DAG: {{\[\[}}intersection(triangle, metal::raytracing::instancing, metal::raytracing::max_levels<3>)]]
// METAL-DAG: metal::array_ref<uint> instanceIndex_{{[0-9]+}} {{\[\[}}instance_id]]
// METAL-DAG: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing, metal::raytracing::max_levels<3>)]]
// METAL-DAG: metal::array_ref<uint> instanceIndex_{{[0-9]+}} {{\[\[}}instance_id]]

// Both candidate dispatchers call the same full-path trie lookup. The count begins after its
// definition and therefore covers one call from each generated intersection function.
// TRIE-LABEL: __slang_structural_rt_instance_contribution_
// TRIE: .size()
// TRIE: instancePath_{{[0-9]+}}{{.*}}[
// TRIE-COUNT-2: __slang_structural_rt_instance_contribution_
