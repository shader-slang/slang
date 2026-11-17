//TEST_IGNORE_FILE:

module generic_target_stage_library;

import slang.raytracing.portable;

namespace stage
{
public interface IMarker {}

public struct Payload
{
    uint value;
}

public struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

public struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::stage::TraceContext;
    typealias Payload = ::stage::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

public __generic<T : IMarker>
extension T : rt::IClosestHitShader
{
    typealias Context = HitContext;
    static void invoke(in rt::ClosestHitInput<Context> input) {}
}
}
