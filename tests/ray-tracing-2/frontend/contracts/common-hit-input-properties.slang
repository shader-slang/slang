//TEST:SIMPLE: -experimental-feature -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -target spirv -no-codegen
//TEST:SIMPLE: -experimental-feature -target cuda -no-codegen
//TEST:SIMPLE: -experimental-feature -target metal -no-codegen

// Check lookup of the shared property implementation after importing the packaged module.
// Each concrete view must keep its native stage contract when using the generic extension.
import slang.raytracing;

struct Payload
{
    float value;
}

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

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(in rt::ClosestHitInput<Context> input)
    {
        input.payload.value = input.minDistance + input.rayFlags + input.primitiveIndex
            + input.geometryIndex + input.worldSpaceOrigin.x + input.worldSpaceDirection.x
            + input.dispatchRaysIndex.x + input.dispatchRaysDimensions.x
            + input.distance + input.hitKind;
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
            + input.distance + input.hitKind;
    }
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(in rt::IntersectionInput<Context> input)
    {
        float candidateDistance = input.minDistance + input.rayFlags + input.primitiveIndex
            + input.geometryIndex + input.worldSpaceOrigin.x + input.worldSpaceDirection.x
            + input.dispatchRaysIndex.x + input.dispatchRaysDimensions.x;
        input.reportHit(candidateDistance, float2(0.0));
    }
}
