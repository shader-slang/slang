//TEST:SIMPLE: -experimental-feature -target metal -no-codegen -DTEST_DEPTH=1
//TEST:SIMPLE: -experimental-feature -target metal -no-codegen -DTEST_DEPTH=2
//TEST:SIMPLE: -experimental-feature -target metal -no-codegen -DTEST_DEPTH=3

// All three hit stages can inspect the actual instance path. These checks cover only the
// declarations; target and runtime tests separately verify synthesized values.
import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<TEST_DEPTH>;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = void;
}

// A caller can constrain the handle family with an ordinary equality without naming the internal
// marker. Both its context and its maximum depth remain generic inside this helper.
uint readGenericPath<C : rt::IHitContext, let maxLevelCount : int>(rt::ClosestHitInput<C> input)
    where C.TraceContext.AccelerationStructure == rt::MultiLevelAccelerationStructure<maxLevelCount>
{
    uint result = input.instanceCount;
    for (uint level = 0; level < input.instanceCount; ++level)
        result += input.getInstanceIndex(level) + input.getInstanceID(level);
    return result;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(in rt::ClosestHitInput<Context> input)
    {
        input.payload.value = readGenericPath<Context, TEST_DEPTH>(input);
        uint count = input.instanceCount;
        for (uint level = 0; level < count; ++level)
        {
            uint index = input.getInstanceIndex(level);
            uint id = input.getInstanceID(level);
            input.payload.value += index + id;
        }
    }
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;

    void invoke(in rt::AnyHitInput<Context> input)
    {
        uint count = input.instanceCount;
        for (uint level = 0; level < count; ++level)
        {
            uint index = input.getInstanceIndex(level);
            uint id = input.getInstanceID(level);
            input.payload.value += index + id;
        }
    }
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(in rt::IntersectionInput<Context> input)
    {
        uint count = input.instanceCount;
        uint total = 0;
        for (uint level = 0; level < count; ++level)
        {
            uint index = input.getInstanceIndex(level);
            uint id = input.getInstanceID(level);
            total += index + id;
        }
        input.reportHit(1.0, float2(float(total)));
    }
}
