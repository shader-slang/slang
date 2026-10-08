//TEST:SIMPLE: -experimental-feature -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -target spirv -no-codegen
//TEST:SIMPLE: -experimental-feature -target cuda -no-codegen
//TEST:SIMPLE: -experimental-feature -target metal -no-codegen
//TEST:SIMPLE: -experimental-feature -target metal -no-codegen -DTEST_MULTILEVEL_DEPTH=1
//TEST:SIMPLE: -experimental-feature -target metal -no-codegen -DTEST_MULTILEVEL_DEPTH=2
//TEST:SIMPLE: -experimental-feature -target metal -no-codegen -DTEST_MULTILEVEL_DEPTH=3

// Check lookup of the shared property implementation after importing the packaged module.
// Each concrete view must keep its native stage contract when using the generic extension.
// Object/world transforms must also be available with direct primitive-AS traversal and with
// multilevel instancing. Backend tests separately exercise the generated Metal adapters.
import slang.raytracing;

struct Payload
{
    float value;
}

struct TraceContext : rt::ITraceContext
{
#ifdef TEST_MULTILEVEL_DEPTH
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<TEST_MULTILEVEL_DEPTH>;
#else
    typealias AccelerationStructure = rt::AccelerationStructure;
#endif
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = void;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(in rt::ClosestHitInput<Context> input)
    {
        input.payload.value = input.minDistance + input.rayFlags + input.primitiveIndex
            + input.geometryIndex + input.worldSpaceOrigin.x + input.worldSpaceDirection.x
            + input.dispatchRaysIndex.x + input.dispatchRaysDimensions.x
            + input.distance + input.hitKind
            + input.objectToWorld[0][0] + input.worldToObject[0][0];
    }
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;

    void invoke(in rt::AnyHitInput<Context> input)
    {
        input.payload.value = input.minDistance + input.rayFlags + input.primitiveIndex
            + input.geometryIndex + input.worldSpaceOrigin.x + input.worldSpaceDirection.x
            + input.dispatchRaysIndex.x + input.dispatchRaysDimensions.x
            + input.distance + input.hitKind
            + input.objectToWorld[0][0] + input.worldToObject[0][0];
    }
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(in rt::IntersectionInput<Context> input)
    {
        float candidateDistance = input.minDistance + input.rayFlags + input.primitiveIndex
            + input.geometryIndex + input.worldSpaceOrigin.x + input.worldSpaceDirection.x
            + input.dispatchRaysIndex.x + input.dispatchRaysDimensions.x
            + input.objectToWorld[0][0] + input.worldToObject[0][0];
        input.reportHit(candidateDistance, float2(0.0));
    }
}
