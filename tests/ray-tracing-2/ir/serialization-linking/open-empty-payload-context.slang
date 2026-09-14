module structural_open_empty_payload_context;

import slang.raytracing;

public struct EmptyPayload
{}

public struct OtherEmptyPayload
{}

public struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

public struct MultilevelTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<3>;
    typealias Motion = rt::NoMotion;
}

public interface IUniqueHitTag : rt::IHitGroup
{}

public interface IUniqueMissTag : rt::IMissShader
{}

public interface IDeduplicatedHitTag : rt::IHitGroup
{}

public interface IAmbiguousMissTag : rt::IMissShader
{}

public interface INotFoundMissTag : rt::IMissShader
{}

public interface IGenericHitTag : rt::IHitGroup
{}

public interface IMultilevelMissTag : rt::IMissShader
{}

public interface ICallableTag : rt::ICallableShader
{}

public struct ListedHitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = EmptyPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

public struct ListedHitGroup : IDeduplicatedHitTag
{
    typealias Context = ListedHitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

public struct ListedMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = EmptyPayload;
    typealias Record = void;
}

public struct ListedMiss : rt::IMissShader
{
    typealias Context = ListedMissContext;
    void invoke(rt::MissInput<Context> input) {}
}

public struct HitOnlySchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::OpenHitGroups<IUniqueHitTag>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

public struct MissOnlySchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::OpenMissShaders<IUniqueMissTag>;
    typealias CallableShaders = rt::NoCallableShaders;
}

public struct SharedSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::OpenHitGroups<IUniqueHitTag>;
    typealias MissShaders = rt::OpenMissShaders<IUniqueMissTag>;
    typealias CallableShaders = rt::NoCallableShaders;
}

public struct DeduplicatedSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::OpenHitGroups<IDeduplicatedHitTag, ListedHitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

public struct AmbiguousSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::OpenMissShaders<IAmbiguousMissTag>;
    typealias CallableShaders = rt::NoCallableShaders;
}

public struct NotFoundSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::OpenMissShaders<INotFoundMissTag>;
    typealias CallableShaders = rt::NoCallableShaders;
}

public struct CallableOnlySchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<ListedMiss>;
    typealias CallableShaders = rt::OpenCallableShaders<ICallableTag>;
}

public struct GenericSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::OpenHitGroups<IGenericHitTag>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

public struct MultilevelSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = MultilevelTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::OpenMissShaders<IMultilevelMissTag>;
    typealias CallableShaders = rt::NoCallableShaders;
}

public void traceHitOnly(
    rt::AccelerationStructure scene,
    rt::TraceProgramDescriptor<HitOnlySchema> program)
{
    rt::RayTracer<HitOnlySchema> tracer;
    tracer.trace({}, scene, program);
}

public void traceMissOnly(
    rt::AccelerationStructure scene,
    rt::TraceProgramDescriptor<MissOnlySchema> program)
{
    rt::RayTracer<MissOnlySchema> tracer;
    tracer.trace({}, scene, program);
}

public void traceShared(
    rt::AccelerationStructure scene,
    rt::TraceProgramDescriptor<SharedSchema> program)
{
    rt::RayTracer<SharedSchema> tracer;
    tracer.trace({}, scene, program);
}

public void traceDeduplicated(
    rt::AccelerationStructure scene,
    rt::TraceProgramDescriptor<DeduplicatedSchema> program)
{
    rt::RayTracer<DeduplicatedSchema> tracer;
    tracer.trace({}, scene, program);
}

public void traceAmbiguous(
    rt::AccelerationStructure scene,
    rt::TraceProgramDescriptor<AmbiguousSchema> program)
{
    rt::RayTracer<AmbiguousSchema> tracer;
    tracer.trace({}, scene, program);
}

public void traceNotFound(
    rt::AccelerationStructure scene,
    rt::TraceProgramDescriptor<NotFoundSchema> program)
{
    rt::RayTracer<NotFoundSchema> tracer;
    tracer.trace({}, scene, program);
}

public void traceCallableOnly(
    rt::AccelerationStructure scene,
    rt::TraceProgramDescriptor<CallableOnlySchema> program)
{
    rt::RayTracer<CallableOnlySchema> tracer;
    tracer.trace({}, scene, program);
}

public void traceGeneric(
    rt::AccelerationStructure scene,
    rt::TraceProgramDescriptor<GenericSchema> program)
{
    rt::RayTracer<GenericSchema> tracer;
    tracer.trace({}, scene, program);
}

public void traceMultilevel(
    rt::MultiLevelAccelerationStructure<3> scene,
    rt::TraceProgramDescriptor<MultilevelSchema> program)
{
    rt::RayTracer<MultilevelSchema> tracer;
    tracer.trace({}, scene, program);
}
