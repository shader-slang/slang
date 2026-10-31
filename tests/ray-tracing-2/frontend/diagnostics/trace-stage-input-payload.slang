//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry Miss -target metal -no-codegen

import slang.raytracing.portable;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Record = void;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;

    static void invoke(rt::MissInput<Context> input)
    {
        rt::RayTracer<Schema> tracer;
        // A compiler-provided stage input may be read by this stage. It cannot become mutable
        // payload storage merely because trace infers its type as an ordinary generic argument.
        tracer.trace({}, scene, program, input);
/*CHECK:
               ^^^^^ ray-tracing stage input cannot be used as a generic type argument
               ^^^^^ ray-tracing stage input type 'rt.MissInput<MissContext>' cannot be used as a generic type argument
*/
    }
}
