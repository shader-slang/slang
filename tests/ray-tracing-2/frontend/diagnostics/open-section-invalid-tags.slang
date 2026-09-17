//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -no-codegen

import slang.raytracing;

struct Payload
{
    uint value;
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
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct CallableData
{
    uint value;
}

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = ::CallableData;
    typealias Record = void;
}

struct ListedClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<Context> input) {}
}

struct ListedHit : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = ListedClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct ListedMiss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<Context> input) {}
}

struct ListedCallable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<Context> input) {}
}

// A normal `Tag : IHitGroup` constraint admits both interface tags and concrete hit groups. Slang
// has no interface-kind constraint that would make every specialization a valid open-section tag,
// so the generic schema itself is rejected instead of leaving an unsafe deferred contract.
struct GenericSchema<Tag> : rt::ITraceProgramSchema
    where Tag : rt::IHitGroup
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::OpenHitGroups<Tag>;
/*CHECK:
              ^^^^^^^^^ invalid structural ray-tracing open-section tag
              ^^^^^^^^^ open hit-group tag 'Tag' must be an interface inheriting 'IHitGroup'
*/
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

// `This` is also unresolved until a concrete type conforms to this interface. Slang gives it a
// dedicated semantic `ThisType`, so source validation must defer it just as it defers `Tag` above.
interface IThisTagSchema : rt::ITraceProgramSchema, rt::IHitGroup
{
    __constraint TraceContext == ::TraceContext;
    __constraint HitGroups == rt::OpenHitGroups<This>;
    __constraint MissShaders == rt::NoMissShaders;
    __constraint CallableShaders == rt::NoCallableShaders;
}

// A generic application needs no special deferral when its outer declaration is an interface:
// substituting `T` cannot turn `IGenericHitTag<T>` into a concrete struct.
interface IGenericHitTag<T> : rt::IHitGroup {}

struct GenericInterfaceSchema<T> : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::OpenHitGroups<IGenericHitTag<T>>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct InvalidSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    // The ordinary generic constraints accept these concrete conformers. Open-section tags are
    // intentionally narrower: the tag itself must be an interface so other conformers can join.
    typealias HitGroups = rt::OpenHitGroups<ListedHit, ListedHit>;
    typealias MissShaders = rt::OpenMissShaders<ListedMiss, ListedMiss>;
    typealias CallableShaders = rt::OpenCallableShaders<ListedCallable, ListedCallable>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<InvalidSchema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<InvalidSchema> tracer;
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    tracer.trace(desc, scene, program, payload);
//CHECK-COUNT-3: error[E38083]: invalid structural ray-tracing open-section tag
//CHECK-DAG: open hit-group tag 'ListedHit' must be an interface inheriting 'IHitGroup'
//CHECK-DAG: open miss-shader tag 'ListedMiss' must be an interface inheriting 'IMissShader'
//CHECK-DAG: open callable-shader tag 'ListedCallable' must be an interface inheriting 'ICallableShader'
}
