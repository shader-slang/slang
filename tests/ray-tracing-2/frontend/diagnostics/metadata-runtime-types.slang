//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestClosestHit : rt::IClosestHitShader<TestHitContext>
{
    void invoke(rt::ClosestHitInput<TestHitContext> input) {}
}

struct TestHitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = TestHitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = rt::NoAnyHit<TestHitContext>;
    typealias Intersection = rt::NoIntersection<TestHitContext>;
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TraceContext, TestHitGroup>;
    typealias MissGroups = rt::NoMissGroups<TraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TraceContext>;
}

void invalidMetadataValues(
    rt::HitGroupSlot<0> slot,
//CHECK:                ^^^^ runtime use of structural ray-tracing metadata
//CHECK:                ^^^^ structural ray-tracing metadata type 'rt.ShaderGroupSlot<0>' has no runtime representation
    TestHitGroup group,
//CHECK:         ^^^^^ runtime use of structural ray-tracing metadata
//CHECK:         ^^^^^ structural ray-tracing metadata type 'TestHitGroup' has no runtime representation
    rt::HitGroupList<TestTraceContext, TestHitGroup> list,
//CHECK:                                             ^^^^ runtime use of structural ray-tracing metadata
//CHECK:                                             ^^^^ structural ray-tracing metadata type 'rt.HitGroupList<TestTraceContext, TestHitGroup>' has no runtime representation
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
