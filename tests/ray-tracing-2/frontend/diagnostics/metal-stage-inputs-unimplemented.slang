//TEST:SIMPLE: -experimental-feature -target metal -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=METAL): -experimental-feature -target metal -whole-program
//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -target hlsl -whole-program
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CUDA): -experimental-feature -target cuda -whole-program -DTEST_CUDA_PENDING_INPUTS=1

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = void;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct ProceduralContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = void;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = void;
}

// Exported helpers keep these bodies reachable during library code generation without selecting
// an entry point. Their ordinary stage-input parameters are legal, and no structural adapter or
// native Metal entry-point check prevents us from testing the pending implementations themselves.
[require(structural_raytracing_closesthit)]
export float readClosestHitState(rt::ClosestHitInput<HitContext> input)
{
    return float(input.rayFlags) + input.objectSpaceRay.Origin.x;
}

[require(structural_raytracing_anyhit)]
export void handleAnyHit(rt::AnyHitInput<HitContext> input)
{
    if (input.objectSpaceRay.Origin.x < 0.0)
        input.ignoreHit();
    else
        input.acceptHitAndEndSearch();
}

[require(structural_raytracing_intersection)]
export bool reportIntersection(rt::IntersectionInput<ProceduralContext> input)
{
    return input.reportHit(1.0, float2(0.0));
}

[require(structural_raytracing_intersection)]
export bool reportIntersectionWithKind(rt::IntersectionInput<ProceduralContext> input)
{
    return input.reportHit(2.0, 7, float2(0.0));
}

// A direct use of the empty triangle view must also reject its pending Metal getter.
[require(structural_raytracing_anyhit_closesthit)]
export bool readFrontFacing()
{
    rt::TriangleData attributes = {};
    return attributes.frontFacing;
}

#ifdef TEST_CUDA_PENDING_INPUTS
struct MotionTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::InstanceMotion;
}

struct MotionHitContext : rt::IHitContext
{
    typealias TraceContext = MotionTraceContext;
    typealias Payload = void;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

// CUDA also has two pending inputs. Keep their exported uses out of the HLSL success case, whose
// traversal API does not provide InstanceMotion.
[require(structural_raytracing_closesthit)]
export uint readGeometryIndex(rt::ClosestHitInput<HitContext> input)
{
    return input.geometryIndex;
}

[require(cuda, structural_raytracing_closesthit)]
export float readRayTime(rt::ClosestHitInput<MotionHitContext> input)
{
    return input.time;
}
#endif

// The packaged module does not retain the assertion messages, so check the diagnostic and the
// getter/method call stacks. Both reportHit overloads must fail independently.
//METAL-DAG: error[E41400]: static assertion failed
//METAL-DAG: see call to 'rayFlags.get'
//METAL-DAG: see call to 'rt.ClosestHitInput.objectSpaceRay.get'
//METAL-DAG: see call to 'rt.AnyHitInput.objectSpaceRay.get'
//METAL-DAG: see call to 'rt.AnyHitInput.ignoreHit'
//METAL-DAG: see call to 'rt.AnyHitInput.acceptHitAndEndSearch'
//METAL-DAG: see call to 'rt.IntersectionInput.reportHit'
//METAL-DAG: see call to 'rt.IntersectionInput.reportHit'
//METAL-DAG: see call to 'rt.TriangleData.frontFacing.get'

//CUDA-DAG: error[E41400]: static assertion failed
//CUDA-DAG: see call to 'geometryIndex.get'
//CUDA-DAG: see call to 'rt.ClosestHitInput.time.get'

// Portable implementations must continue to emit their native intrinsics.
//HLSL-DAG: RayFlags()
//HLSL-DAG: ObjectRayOrigin()
//HLSL-DAG: IgnoreHit()
//HLSL-DAG: AcceptHitAndEndSearch()
//HLSL-DAG: ReportHit(
//HLSL-DAG: HitKind()
