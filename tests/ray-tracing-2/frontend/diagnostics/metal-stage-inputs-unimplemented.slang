//TEST:SIMPLE: -experimental-feature -target metal -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=METAL): -experimental-feature -target metal -whole-program
//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -target hlsl -whole-program
//DIAGNOSTIC_TEST:SIMPLE(diag=CUDA): -experimental-feature -target cuda -whole-program -DTEST_CUDA_PENDING_INPUTS=1

import slang.raytracing.portable;

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
// The packaged library supplies no source locations for these errors or call notes. Keep the
// message checks beside the calls that reach each unfinished operation.
[require(portable_raytracing_closesthit)]
export float readClosestHitState(rt::ClosestHitInput<HitContext> input)
{
    return float(input.rayFlags) + input.objectSpaceRay.Origin.x;
//METAL: error E41400
//METAL: Metal structural ray-tracing rayFlags
//METAL: see call to 'rayFlags.get'
//METAL: error E41400
//METAL: Metal structural ray-tracing objectSpaceRay
//METAL: see call to 'rt.ClosestHitInput.objectSpaceRay.get'
}

[require(portable_raytracing_anyhit)]
export void handleAnyHit(rt::AnyHitInput<HitContext> input)
{
    if (input.objectSpaceRay.Origin.x < 0.0)
//METAL: error E41400
//METAL: Metal structural ray-tracing objectSpaceRay
//METAL: see call to 'rt.AnyHitInput.objectSpaceRay.get'
        input.ignoreHit();
//METAL: error E41400
//METAL: Metal structural ray-tracing ignoreHit
//METAL: see call to 'rt.AnyHitInput.ignoreHit'
    else
        input.acceptHitAndEndSearch();
//METAL: error E41400
//METAL: Metal structural ray-tracing acceptHitAndEndSearch
//METAL: see call to 'rt.AnyHitInput.acceptHitAndEndSearch'
}

[require(portable_raytracing_intersection)]
export bool reportIntersection(rt::IntersectionInput<ProceduralContext> input)
{
    return input.reportHit(1.0, float2(0.0));
//METAL: error E41400
//METAL: Metal structural ray-tracing reportHit
//METAL: see call to 'rt.IntersectionInput.reportHit'
}

[require(portable_raytracing_intersection)]
export bool reportIntersectionWithKind(rt::IntersectionInput<ProceduralContext> input)
{
    return input.reportHit(2.0, 7, float2(0.0));
//METAL: error E41400
//METAL: Metal structural ray-tracing reportHit
//METAL: see call to 'rt.IntersectionInput.reportHit'
}

// A direct use of the empty triangle view must also reject its pending Metal getter.
[require(portable_raytracing_anyhit_closesthit)]
export bool readFrontFacing()
{
    rt::TriangleData attributes = {};
    return attributes.frontFacing;
//METAL: error E41400
//METAL: Metal structural ray-tracing frontFacing
//METAL: see call to 'rt.TriangleData.frontFacing.get'
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
[require(portable_raytracing_closesthit)]
export uint readGeometryIndex(rt::ClosestHitInput<HitContext> input)
{
    return input.geometryIndex;
//CUDA: error E41400
//CUDA: CUDA structural ray-tracing geometryIndex
//CUDA: see call to 'geometryIndex.get'
}

[require(cuda, portable_raytracing_closesthit)]
export float readRayTime(rt::ClosestHitInput<MotionHitContext> input)
{
    return input.time;
//CUDA: error E41400
//CUDA: CUDA structural ray-tracing time
//CUDA: see call to 'rt.ClosestHitInput.time.get'
}
#endif

// Portable implementations must continue to emit their native intrinsics.
//HLSL-DAG: RayFlags()
//HLSL-DAG: ObjectRayOrigin()
//HLSL-DAG: IgnoreHit()
//HLSL-DAG: AcceptHitAndEndSearch()
//HLSL-DAG: ReportHit(
//HLSL-DAG: HitKind()
