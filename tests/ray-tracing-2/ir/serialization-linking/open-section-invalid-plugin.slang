module structural_open_section_invalid_plugin;

import slang.raytracing;
import structural_open_section_context;

// This entry is independently well-formed, but it belongs to a different trace context from the
// schema that selects IHitTag. That cross-module relationship is intentionally left for the linked
// schema boundary to diagnose.
struct ForeignTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct ForeignHitContext : rt::IHitContext
{
    typealias TraceContext = ForeignTraceContext;
    typealias Payload = LinkedPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct ForeignClosestHit : rt::IClosestHitShader
{
    typealias Context = ForeignHitContext;
    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.value += 1;
    }
}

struct ForeignHitGroup : IHitTag
{
    typealias Context = ForeignHitContext;
    typealias ClosestHit = ForeignClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct FirstCallableContext : rt::ICallableContext
{
    typealias TraceContext = SharedTraceContext;
    typealias CallableData = uint;
    typealias Record = void;
}

struct SecondCallableContext : rt::ICallableContext
{
    typealias TraceContext = SharedTraceContext;
    typealias CallableData = float;
    typealias Record = void;
}

struct FirstCallable : ICallableTag
{
    typealias Context = FirstCallableContext;
    void invoke(rt::CallableInput<Context> input) {}
}

struct SecondCallable : ICallableTag
{
    typealias Context = SecondCallableContext;
    void invoke(rt::CallableInput<Context> input) {}
}
