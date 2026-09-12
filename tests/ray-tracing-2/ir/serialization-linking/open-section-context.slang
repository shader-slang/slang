module structural_open_section_context;

import slang.raytracing;

public struct ListedPayload
{
    public uint value;
}

public struct LinkedPayload
{
    public uint value;
}

public struct SharedTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

public struct ListedHitContext : rt::IHitContext
{
    typealias TraceContext = SharedTraceContext;
    typealias Payload = ListedPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

public struct LinkedHitContext : rt::IHitContext
{
    typealias TraceContext = SharedTraceContext;
    typealias Payload = LinkedPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

public struct ListedMissContext : rt::IPayloadContext
{
    typealias TraceContext = SharedTraceContext;
    typealias Payload = ListedPayload;
    typealias Record = void;
}

public struct LinkedMissContext : rt::IPayloadContext
{
    typealias TraceContext = SharedTraceContext;
    typealias Payload = LinkedPayload;
    typealias Record = void;
}

public struct SharedCallableData
{
    public uint value;
}

public struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = SharedTraceContext;
    typealias CallableData = SharedCallableData;
    typealias Record = void;
}

public interface IHitTag : rt::IHitGroup
{}

public interface IDerivedHitTag : IHitTag
{}

public interface IMissTag : rt::IMissShader
{}

public interface ICallableTag : rt::ICallableShader
{}
