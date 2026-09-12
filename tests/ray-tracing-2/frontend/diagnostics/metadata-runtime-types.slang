//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities

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
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

void invalidMetadataValues(
    TestHitGroup group,
//CHECK:         ^^^^^ runtime use of structural ray-tracing metadata
//CHECK:         ^^^^^ structural ray-tracing metadata type 'TestHitGroup' has no runtime representation
    rt::HitGroupList<TestHitGroup> hitGroups,
//CHECK:                           ^^^^^^^^^ runtime use of structural ray-tracing metadata
//CHECK:                           ^^^^^^^^^ structural ray-tracing metadata type 'rt.HitGroupList<TestHitGroup>' has no runtime representation
    rt::NoMissShaders missShaders,
//CHECK:              ^^^^^^^^^^^ runtime use of structural ray-tracing metadata
//CHECK:              ^^^^^^^^^^^ structural ray-tracing metadata type 'rt.NoMissShaders' has no runtime representation
    rt::NoCallableShaders callableShaders,
//CHECK:                  ^^^^^^^^^^^^^^^ runtime use of structural ray-tracing metadata
//CHECK:                  ^^^^^^^^^^^^^^^ structural ray-tracing metadata type 'rt.NoCallableShaders' has no runtime representation
    TestSchema schema,
//CHECK:       ^^^^^^ runtime use of structural ray-tracing metadata
//CHECK:       ^^^^^^ structural ray-tracing metadata type 'TestSchema' has no runtime representation
    rt::ITraceProgramSchema schemaInterface)
//CHECK:                    ^^^^^^^^^^^^^^^ runtime use of structural ray-tracing metadata
//CHECK:                    ^^^^^^^^^^^^^^^ structural ray-tracing metadata type 'rt.ITraceProgramSchema' has no runtime representation
{
    TestSchema();
/*CHECK:
    ^^^^^^^^^^ construction of a structural ray-tracing type
    ^^^^^^^^^^ structural ray-tracing type 'TestSchema' cannot be constructed by user code
*/
}
