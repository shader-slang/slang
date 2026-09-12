//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct Payload
{
    uint value;
}

struct CallableData
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

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = ::TraceContext;
    typealias CallableData = ::CallableData;
    typealias Record = void;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<Context> input)
    {
        input.payload.value++;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = ::ClosestHit;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct Miss : rt::IMissShader
{
    typealias Context = HitContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value++;
    }
}

struct Callable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<Context> input)
    {
        input.data.value++;
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::CallableShaderList<Callable>;
}

rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<Schema> tracer;
    CallableData data = {};
    // This callable dispatch is the only runtime operation that activates `Schema`. The physical
    // descriptor must nevertheless contain the complete hit, miss, and callable program so host
    // reflection and generated Metal symbols describe the same schema.
    tracer.callShader<CallableContext>(0, program, data);
}

// METAL: metal::raytracing::intersection_function_table<
// METAL: metal::visible_function_table<{{.*}}> missFunctions
// METAL: metal::visible_function_table<{{.*}}> closestHitFunctions
// METAL: metal::visible_function_table<{{.*}}> callableFunctions
// METAL: {{\[\[visible\]\]}} void {{.*}}(CallableData_0 thread*
// METAL: {{\[\[visible\]\]}} void {{.*}}(Schema_rayData_0 thread*
// METAL: {{\[\[visible\]\]}} void {{.*}}(Schema_rayData_0 thread*
