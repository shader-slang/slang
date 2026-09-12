module payload_location_cross_module_support;

import slang.raytracing;

public struct FirstTag {}
public struct SecondTag {}

// These specializations have identical storage but distinct canonical semantic identities.
public struct PayloadValue<Tag>
{
    public uint value;
}

public typealias FirstPayloadAlias = PayloadValue<FirstTag>;

public struct SharedTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

public struct FirstMissContext : rt::IPayloadContext
{
    typealias TraceContext = SharedTraceContext;
    typealias Payload = PayloadValue<FirstTag>;
    typealias Record = void;
}

public struct SecondMissContext : rt::IPayloadContext
{
    typealias TraceContext = SharedTraceContext;
    typealias Payload = PayloadValue<SecondTag>;
    typealias Record = void;
}

public struct FirstMiss : rt::IMissShader
{
    typealias Context = FirstMissContext;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value += 1;
    }
}

public struct SecondMiss : rt::IMissShader
{
    typealias Context = SecondMissContext;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value += 1;
    }
}

public struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = SharedTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<FirstMiss, SecondMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}
