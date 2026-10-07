//TEST:SIMPLE(filecheck=ZERO): -experimental-feature -target metal -whole-program -DTEST_DEPTH=1 -DTEST_MAX_INSTANCES=0
//TEST:SIMPLE(filecheck=ONE): -experimental-feature -target metal -whole-program -DTEST_DEPTH=2 -DTEST_MAX_INSTANCES=1
//TEST:SIMPLE(filecheck=TWO): -experimental-feature -target metal -whole-program -DTEST_DEPTH=3 -DTEST_MAX_INSTANCES=2

// The static limit must fold through the packaged module's associated constant and generic
// extension. Only constants reach code generation; reading a hit's actual path still needs the
// pending Metal adapter. One acceleration-structure level is occupied by the primitive AS.
// The generic closest-hit helper and the two other stages must all emit the expected constant.
//ZERO-DAG: return 0U;
//ZERO-DAG: return 0U;
//ZERO-DAG: return 0U;
//ONE-DAG: return 1U;
//ONE-DAG: return 1U;
//ONE-DAG: return 1U;
//TWO-DAG: return 2U;
//TWO-DAG: return 2U;
//TWO-DAG: return 2U;
import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<TEST_DEPTH>;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = void;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = void;
}

uint readGenericLimit<C : rt::IHitContext, let depth : int>()
    where C.TraceContext.AccelerationStructure == rt::MultiLevelAccelerationStructure<depth>
{
    // Include a sentinel slot so direct primitive-AS traversal does not need a zero-length array.
    uint storage[rt::ClosestHitInput<C>.maxInstanceCount + 1] = {};
    static_assert(storage.getCount() == depth, "The primitive AS occupies one additional level.");
    return rt::ClosestHitInput<C>.maxInstanceCount;
}

[require(metal, structural_raytracing_closesthit)]
export uint closestHitLimit()
{
    static_assert(TraceContext.AccelerationStructure.levelCount == TEST_DEPTH, "AS level count");
    static_assert(
        rt::ClosestHitInput<HitContext>.maxInstanceCount == TEST_MAX_INSTANCES,
        "Closest-hit instance limit");
    return readGenericLimit<HitContext, TEST_DEPTH>();
}

[require(metal, structural_raytracing_anyhit)]
export uint anyHitLimit()
{
    static_assert(
        rt::AnyHitInput<HitContext>.maxInstanceCount == TEST_MAX_INSTANCES,
        "Any-hit instance limit");
    return rt::AnyHitInput<HitContext>.maxInstanceCount;
}

[require(metal, structural_raytracing_intersection)]
export uint intersectionLimit()
{
    static_assert(
        rt::IntersectionInput<HitContext>.maxInstanceCount == TEST_MAX_INSTANCES,
        "Intersection instance limit");
    return rt::IntersectionInput<HitContext>.maxInstanceCount;
}
