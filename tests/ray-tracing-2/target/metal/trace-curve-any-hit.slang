//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_6

import slang.raytracing;

struct TestPayload
{
    float parameter;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::CurvePrimitive;
    typealias Record = TestRecord;
}

struct TestAnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        input.payload.parameter = input.curve.parameter;
        if (input.curve.parameter < 0.5)
            input.ignoreHit();
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = TestAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Layout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestTraceContext, HitGroup>;
    typealias MissGroups = rt::NoMissGroups<TestTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Layout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    TestPayload payload = {};
    rt::RayTracer<Layout> tracer;
    tracer.trace(desc, scene, program, payload);
//CHECK:        ^ structural ray-tracing curve primitives require Metal
//CHECK:        ^ 'CurvePrimitive' is only supported when compiling the structural ray-tracing API for Metal
}

// METAL: intersection_function_table<metal::raytracing::instancing, metal::raytracing::curve_data>
// METAL: assume_geometry_type(metal::raytracing::geometry_type::curve)
// METAL: {{\[\[}}intersection(curve, metal::raytracing::instancing, metal::raytracing::curve_data)]]
// METAL-SAME: float curveParameter_{{[0-9]+}} {{\[\[}}curve_parameter]]
// METAL: parameter_0 = curveParameter_
