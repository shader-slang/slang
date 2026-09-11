//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities

import slang.raytracing;

struct PayloadA {}
struct PayloadB {}
struct Record {}

struct TraceContextA : rt::ITraceContext
{
    typealias Payload = PayloadA;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TraceContextB : rt::ITraceContext
{
    typealias Payload = PayloadB;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContextA : rt::IHitContext
{
    typealias TraceContext = TraceContextA;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = Record;
}

struct HitContextB : rt::IHitContext
{
    typealias TraceContext = TraceContextB;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = Record;
}

struct MissContextA : rt::IMissGroupContext
{
    typealias TraceContext = TraceContextA;
    typealias Record = Record;
}

struct MissContextB : rt::IMissGroupContext
{
    typealias TraceContext = TraceContextB;
    typealias Record = Record;
}

struct CallableData {}

struct CallableContextA : rt::ICallableGroupContext
{
    typealias TraceContext = TraceContextA;
    typealias CallableData = CallableData;
    typealias Record = Record;
}

struct CallableContextB : rt::ICallableGroupContext
{
    typealias TraceContext = TraceContextB;
    typealias CallableData = CallableData;
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
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContextA;
    typealias ClosestHit = ClosestHitB;
    typealias AnyHit = rt::NoAnyHit<HitContextA>;
    typealias Intersection = rt::NoIntersection<HitContextA>;
}

typealias ForceHitCheck = rt::HitGroupList<TraceContextA, BadHitGroup>;

// The any-hit stage has an independent context-agreement requirement.
struct BadAnyHitContextGroup : rt::IHitGroup
//CHECK:                           ^^^^^^^^^ type argument doesn't conform to interface
//CHECK:                           ^^^^^^^^^ type argument 'BadAnyHitContextGroup' does not conform to the required interface 'rt.IHitGroup'
{
    typealias Slot = rt::HitGroupSlot<1>;
    typealias Context = HitContextA;
    typealias ClosestHit = rt::NoClosestHit<HitContextA>;
    typealias AnyHit = rt::NoAnyHit<HitContextB>;
    typealias Intersection = rt::NoIntersection<HitContextA>;
}

typealias ForceAnyHitCheck = rt::HitGroupList<TraceContextA, BadAnyHitContextGroup>;

// The intersection stage has an independent context-agreement requirement.
struct BadIntersectionContextGroup : rt::IHitGroup
//CHECK:                                 ^^^^^^^^^ type argument doesn't conform to interface
//CHECK:                                 ^^^^^^^^^ type argument 'BadIntersectionContextGroup' does not conform to the required interface 'rt.IHitGroup'
{
    typealias Slot = rt::HitGroupSlot<2>;
    typealias Context = HitContextA;
    typealias ClosestHit = rt::NoClosestHit<HitContextA>;
    typealias AnyHit = rt::NoAnyHit<HitContextA>;
    typealias Intersection = rt::NoIntersection<HitContextB>;
}

typealias ForceIntersectionCheck =
    rt::HitGroupList<TraceContextA, BadIntersectionContextGroup>;

struct MissB : rt::IMissShader
{
    typealias Context = MissContextB;
    void invoke(rt::MissInput<MissContextB> input) {}
}

// A miss group and its stage must expose the same context.
struct BadMissGroup : rt::IMissGroup
//CHECK:                  ^^^^^^^^^^ type argument doesn't conform to interface
//CHECK:                  ^^^^^^^^^^ type argument 'BadMissGroup' does not conform to the required interface 'rt.IMissGroup'
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = MissContextA;
    typealias Miss = MissB;
}

typealias ForceMissCheck = rt::MissGroupList<TraceContextA, BadMissGroup>;

struct CallableB : rt::ICallableShader
{
    typealias Context = CallableContextB;
    void invoke(rt::CallableInput<CallableContextB> input) {}
}

// A callable group and its stage must expose the same context.
struct BadCallableGroup : rt::ICallableGroup
//CHECK:                      ^^^^^^^^^^^^^^ type argument doesn't conform to interface
//CHECK:                      ^^^^^^^^^^^^^^ type argument 'BadCallableGroup' does not conform to the required interface 'rt.ICallableGroup'
{
    typealias Slot = rt::CallableSlot<0>;
    typealias Context = CallableContextA;
    typealias Callable = CallableB;
}

typealias ForceCallableCheck = rt::CallableGroupList<TraceContextA, BadCallableGroup>;
