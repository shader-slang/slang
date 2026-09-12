module structural_stage_name_collision_common;

import slang.raytracing;

public struct SharedPayload
{
    public uint value;
}

public struct SharedTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

public struct SharedMissContext : rt::IPayloadContext
{
    typealias TraceContext = SharedTraceContext;
    typealias Payload = SharedPayload;
    typealias Record = void;
}
