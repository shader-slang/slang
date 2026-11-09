//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

struct Resources
{
    RWStructuredBuffer<uint> values;
}

[noinline]
Optional<RWStructuredBuffer<uint>> getNoBuffer()
{
    return none;
}

[noinline]
Optional<Resources> getNoResources()
{
    return none;
}

[noinline]
Optional<RWStructuredBuffer<uint>> castToOptional(RWStructuredBuffer<uint> buffer)
{
    return buffer as RWStructuredBuffer<uint>;
}

[host]
public bool main()
{
    Optional<RWStructuredBuffer<uint>> buffer = getNoBuffer();
    Optional<Resources> resources = getNoResources();
    return buffer.hasValue || resources.hasValue;
}

// CHECK: Section("LITR")
// CHECK: [[NULL_BUFFER:%[A-Za-z0-9_]+]] : [[BUFFER:%[A-Za-z0-9_]+]] = OpNullLit
// CHECK: Section("TYPE")
// CHECK-DAG: [[BUFFER]] = OpRWStructuredBufferType
// CHECK: Section("FUNC")
// CHECK-DAG: OpMakeStruct [[NULL_BUFFER]]
