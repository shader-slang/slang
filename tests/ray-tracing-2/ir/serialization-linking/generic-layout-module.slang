module structural_generic_layout;

import slang.raytracing;

public struct GenericPayload<T>
{
    public T value;
}

public struct GenericTraceContext<T> : rt::ITraceContext
{
    typealias Payload = GenericPayload<T>;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

public struct GenericMissContext<T> : rt::IMissGroupContext
{
    typealias TraceContext = GenericTraceContext<T>;
    typealias Record = void;
}

public struct GenericMiss<T> : rt::IMissShader<GenericMissContext<T>>
{
    void invoke(rt::MissInput<GenericMissContext<T>> input)
    {
        input.payload.value = input.payload.value;
    }
}

public struct GenericMissGroup<T> : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = GenericMissContext<T>;
    typealias Miss = GenericMiss<T>;
}

public struct GenericProgramLayout<T> : rt::ITraceProgramLayout
{
    typealias TraceContext = GenericTraceContext<T>;
    typealias HitGroups = rt::NoHitGroups<GenericTraceContext<T>>;
    typealias MissGroups = rt::MissGroupList<GenericTraceContext<T>, GenericMissGroup<T>>;
    typealias CallableGroups = rt::NoCallableGroups<GenericTraceContext<T>>;
}
