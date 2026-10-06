//TEST:SIMPLE: -experimental-feature -target metal -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -target metal -whole-program

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<3>;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = void;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = void;
}

// Exported helpers reach the pending getters during ordinary library code generation, without
// selecting a structural entry point whose separate unimplemented-adapter diagnostic would hide
// these assertions. All three public operations must fail instead of producing placeholder IDs.
[require(metal, structural_raytracing_closesthit)]
export uint readInstanceCount(rt::ClosestHitInput<HitContext> input)
{
    return input.instanceCount;
}

[require(metal, structural_raytracing_anyhit)]
export uint readInstanceIndex(rt::AnyHitInput<HitContext> input, uint level)
{
    return input.getInstanceIndex(level);
}

[require(metal, structural_raytracing_intersection)]
export uint readInstanceID(rt::IntersectionInput<HitContext> input, uint level)
{
    return input.getInstanceID(level);
}

//CHECK-DAG: error[E41400]: static assertion failed
//CHECK-DAG: see call to 'rt.ClosestHitInput.instanceCount.get'
//CHECK-DAG: see call to 'rt.AnyHitInput.getInstanceIndex'
//CHECK-DAG: see call to 'rt.IntersectionInput.getInstanceID'
