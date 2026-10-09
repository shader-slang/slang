//TEST:SIMPLE: -experimental-feature -ignore-capabilities -entry main -stage compute -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -ignore-capabilities -entry main -stage compute -target metal -no-codegen

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestRecord
{
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestHitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestHitGroup>;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

void consume<T>(T value) {}

// Schemas, hit groups, and shader lists use ordinary generic substitution. Neither their
// associated types nor their type-pack arguments require constructing a shader-stage value.
struct Phantom<T> {}

typealias SchemaPhantom = Phantom<TestSchema>;
typealias HitGroupPhantom = Phantom<TestHitGroup>;
typealias ListPhantom = Phantom<rt::HitGroupList<TestHitGroup>>;
typealias InterfacePhantom = Phantom<rt::ITraceProgramSchema>;
typealias LegalTracer = rt::RayTracer<TestSchema>;
typealias LegalDescriptor = rt::TraceProgramDescriptor<TestSchema>;

void useType<T>() {}
void useTypes<each T>() {}

[numthreads(1, 1, 1)]
void main()
{
    useType<TestHitGroup>();
    useType<TestSchema>();
    useTypes<rt::HitGroupList<TestHitGroup>, rt::EmptyMissShaderList,
        rt::EmptyCallableShaderList, TestSchema>();

    SchemaPhantom schemaTag = {};
    HitGroupPhantom hitGroupTag = {};
    ListPhantom listTag = {};
    InterfacePhantom interfaceTag = {};
    consume(schemaTag);
    consume(hitGroupTag);
    consume(listTag);
    consume(interfaceTag);
}
