//TEST:SIMPLE: -experimental-feature -ignore-capabilities -no-codegen

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = TestHitContext;
    void invoke(rt::ClosestHitInput<TestHitContext> input) {}
}

struct TestHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = rt::NoAnyHit<TestHitContext>;
    typealias Intersection = rt::NoIntersection<TestHitContext>;
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestHitGroup>;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

struct Box<T>
{
    T value;
}

// Associated types do not create instance fields. Schema/list/group values can be copied,
// returned, nested in ordinary containers, and consumed through ordinary generic constraints.
T copyValue<T>(T value) { return value; }

TestSchema copySchema(TestSchema schema) { return copyValue(schema); }

void acceptMetadataValues(
    TestHitGroup group,
    rt::HitGroupList<TestHitGroup> hitGroups,
    rt::EmptyMissShaderList missShaders,
    rt::EmptyCallableShaderList callableShaders,
    TestSchema schema,
    Box<Box<TestHitGroup>> nestedGroup,
    rt::ITraceProgramSchema schemaInterface)
{
    TestSchema constructedSchema = {};
    Box<Box<TestHitGroup>> constructedGroup = {};
    copySchema(constructedSchema);
    copyValue(constructedGroup);
    copyValue(group);
    copyValue(hitGroups);
    copyValue(missShaders);
    copyValue(callableShaders);
    copyValue(schemaInterface);
}
