module structural_generic_schema;

import slang.raytracing;

public struct GenericPayload<T>
{
    public T value;
}

public struct GenericTraceContext<T> : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

public struct GenericMissContext<T> : rt::IPayloadContext
{
    typealias TraceContext = GenericTraceContext<T>;
    typealias Payload = GenericPayload<T>;
    typealias Record = void;
}

public struct GenericMiss<T> : rt::IMissShader
{
    typealias Context = GenericMissContext<T>;
    void invoke(rt::MissInput<GenericMissContext<T>> input)
    {
        input.payload.value = input.payload.value;
    }
}

public struct GenericSchema<T> : rt::ITraceProgramSchema
{
    typealias TraceContext = GenericTraceContext<T>;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<GenericMiss<T>>;
    typealias CallableShaders = rt::NoCallableShaders;
}

[noinline]
public rt::TraceProgramDescriptor<GenericSchema<T>> preserveProgram<T>(
    rt::TraceProgramDescriptor<GenericSchema<T>> program)
{
    // This generic helper is serialized before its schema argument is known. The compiler-owned
    // descriptor type must retain that argument so importing code can specialize this one body for
    // multiple schemas without merging their physical descriptor types.
    return program;
}
