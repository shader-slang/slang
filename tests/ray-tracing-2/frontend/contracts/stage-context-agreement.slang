//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities

import slang.raytracing;

struct PayloadA {}
struct PayloadB {}
struct Record {}

struct TraceContextA : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TraceContextB : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContextA : rt::IHitContext
{
    typealias TraceContext = TraceContextA;
    typealias Payload = PayloadA;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = Record;
}

struct HitContextB : rt::IHitContext
{
    typealias TraceContext = TraceContextB;
    typealias Payload = PayloadB;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = Record;
}

struct ClosestHitB : rt::IClosestHitShader
{
    typealias Context = HitContextB;
    void invoke(rt::ClosestHitInput<HitContextB> input) {}
}

// A hit group cannot combine a stage using HitContextB with the group's HitContextA.
struct BadHitGroup : rt::IHitGroup
//CHECK:                 ^^^^^^^^^ type argument doesn't conform to interface
//CHECK:                 ^^^^^^^^^ type argument 'BadHitGroup' does not conform to the required interface 'rt.IHitGroup'
{
    typealias Context = HitContextA;
    typealias ClosestHit = ClosestHitB;
    typealias AnyHit = rt::NoAnyHit<HitContextA>;
    typealias Intersection = rt::NoIntersection<HitContextA>;
}

typealias ForceHitCheck = rt::HitGroupList<BadHitGroup>;

// The any-hit stage has an independent context-agreement requirement.
struct BadAnyHitContextGroup : rt::IHitGroup
//CHECK:                           ^^^^^^^^^ type argument doesn't conform to interface
//CHECK:                           ^^^^^^^^^ type argument 'BadAnyHitContextGroup' does not conform to the required interface 'rt.IHitGroup'
{
    typealias Context = HitContextA;
    typealias ClosestHit = rt::NoClosestHit<HitContextA>;
    typealias AnyHit = rt::NoAnyHit<HitContextB>;
    typealias Intersection = rt::NoIntersection<HitContextA>;
}

typealias ForceAnyHitCheck = rt::HitGroupList<BadAnyHitContextGroup>;

// The intersection stage has an independent context-agreement requirement.
struct BadIntersectionContextGroup : rt::IHitGroup
//CHECK:                                 ^^^^^^^^^ type argument doesn't conform to interface
//CHECK:                                 ^^^^^^^^^ type argument 'BadIntersectionContextGroup' does not conform to the required interface 'rt.IHitGroup'
{
    typealias Context = HitContextA;
    typealias ClosestHit = rt::NoClosestHit<HitContextA>;
    typealias AnyHit = rt::NoAnyHit<HitContextA>;
    typealias Intersection = rt::NoIntersection<HitContextB>;
}

typealias ForceIntersectionCheck = rt::HitGroupList<BadIntersectionContextGroup>;
