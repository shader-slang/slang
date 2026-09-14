module structural_open_empty_payload_plugin_a;

import slang.raytracing;
import structural_open_empty_payload_context;

struct UniqueHitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = EmptyPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct UniqueClosestHit : rt::IClosestHitShader
{
    typealias Context = UniqueHitContext;
    void invoke(rt::ClosestHitInput<Context> input) {}
}

struct UniqueHitGroup : IUniqueHitTag
{
    typealias Context = UniqueHitContext;
    typealias ClosestHit = UniqueClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct UniqueMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = EmptyPayload;
    typealias Record = void;
}

struct UniqueMiss : IUniqueMissTag
{
    typealias Context = UniqueMissContext;
    void invoke(rt::MissInput<Context> input) {}
}

struct MultilevelMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::MultilevelTraceContext;
    typealias Payload = EmptyPayload;
    typealias Record = void;
}

struct MultilevelMiss : IMultilevelMissTag
{
    typealias Context = MultilevelMissContext;
    void invoke(rt::MissInput<Context> input) {}
}

struct DeduplicatedHitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = EmptyPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct DeduplicatedHitGroup : IDeduplicatedHitTag
{
    typealias Context = DeduplicatedHitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct AmbiguousMissContextA : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = EmptyPayload;
    typealias Record = void;
}

struct AmbiguousMissA : IAmbiguousMissTag
{
    typealias Context = AmbiguousMissContextA;
    void invoke(rt::MissInput<Context> input) {}
}

public struct GenericHitContext<T> : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = EmptyPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

public struct GenericClosestHit<T> : rt::IClosestHitShader
{
    typealias Context = GenericHitContext<T>;
    void invoke(rt::ClosestHitInput<Context> input) {}
}

public struct GenericHitGroup<T> : IGenericHitTag
{
    typealias Context = GenericHitContext<T>;
    typealias ClosestHit = GenericClosestHit<T>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}
