//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -no-codegen

import slang.raytracing;

struct Payload {}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = void;
}

// The shared interface must not expose hit-only properties to an intersection stage. It has not
// reported an accepted hit yet, so it cannot read the corresponding distance or classification.
struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(in rt::IntersectionInput<Context> input)
    {
        let distance = input.distance;
//CHECK-DAG: 'distance' is not a member of 'rt.IntersectionInput<HitContext>'.
        let hitKind = input.hitKind;
//CHECK-DAG: 'hitKind' is not a member of 'rt.IntersectionInput<HitContext>'.
    }
}

// A miss has no hit primitive, so it does not acquire the common hit input properties even when
// its payload context happens to include a primitive type.
struct Miss : rt::IMissShader
{
    typealias Context = HitContext;

    void invoke(in rt::MissInput<Context> input)
    {
        let primitiveIndex = input.primitiveIndex;
//CHECK-DAG: 'primitiveIndex' is not a member of 'rt.MissInput<HitContext>'.
        let objectToWorld = input.objectToWorld;
//CHECK-DAG: 'objectToWorld' is not a member of 'rt.MissInput<HitContext>'.
        let worldToObject = input.worldToObject;
//CHECK-DAG: 'worldToObject' is not a member of 'rt.MissInput<HitContext>'.
    }
}

// Sharing implementations must not create an erased public runtime stage-view interface.
struct FakeCommonHitInput : rt::ICommonHitInput {}
//CHECK-DAG: 'ICommonHitInput' is not accessible from the current context.
