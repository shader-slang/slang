//TEST:SIMPLE: -experimental-feature -target metal -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -target metal -whole-program

import slang.raytracing.portable;

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
// The packaged library reports errors and call notes without source locations; keep each
// message check beside the corresponding call.
[require(metal, structural_raytracing_closesthit)]
export uint readInstanceCount(rt::ClosestHitInput<HitContext> input)
{
    return input.instanceCount;
//CHECK: error E41400
//CHECK: Metal structural ray-tracing instanceCount requires compiler synthesis
//CHECK: see call to 'rt.ClosestHitInput.instanceCount.get'
}

[require(metal, structural_raytracing_anyhit)]
export uint readInstanceIndex(rt::AnyHitInput<HitContext> input, uint level)
{
    return input.getInstanceIndex(level);
//CHECK: error E41400
//CHECK: Metal structural ray-tracing getInstanceIndex requires compiler synthesis
//CHECK: see call to 'rt.AnyHitInput.getInstanceIndex'
}

[require(metal, structural_raytracing_intersection)]
export uint readInstanceID(rt::IntersectionInput<HitContext> input, uint level)
{
    return input.getInstanceID(level);
//CHECK: error E41400
//CHECK: Metal structural ray-tracing getInstanceID requires compiler synthesis
//CHECK: see call to 'rt.IntersectionInput.getInstanceID'
}
