//TEST:SIMPLE: -experimental-feature -entry main -stage compute -no-codegen

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

struct TestLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TraceContext, TestHitGroup>;
    typealias MissGroups = rt::NoMissGroups<TraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TraceContext>;
}

[numthreads(1, 1, 1)]
void main() {}
