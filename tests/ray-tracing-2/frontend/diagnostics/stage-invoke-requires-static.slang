//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

// All five stage interfaces require static invoke. An otherwise matching instance method must
// fail ordinary interface conformance, even if no stage is selected as an entry point.

import slang.raytracing.portable;

struct TestPayload {}
struct TestRecord {}
struct TestAttributes {}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<TestAttributes>;
    typealias Record = TestRecord;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Record = TestRecord;
}

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = uint;
    typealias Record = TestRecord;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input) {}
//CHECK: ^^^^^^ interface requirement mismatch
//CHECK: ^^^^^^ member 'invoke' does not match interface requirement.
//CHECK: see interface requirement declaration of 'invoke'
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input) {}
//CHECK: ^^^^^^ interface requirement mismatch
//CHECK: ^^^^^^ member 'invoke' does not match interface requirement.
//CHECK: see interface requirement declaration of 'invoke'
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;
    void invoke(rt::IntersectionInput<HitContext> input) {}
//CHECK: ^^^^^^ interface requirement mismatch
//CHECK: ^^^^^^ member 'invoke' does not match interface requirement.
//CHECK: see interface requirement declaration of 'invoke'
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input) {}
//CHECK: ^^^^^^ interface requirement mismatch
//CHECK: ^^^^^^ member 'invoke' does not match interface requirement.
//CHECK: see interface requirement declaration of 'invoke'
}

struct Callable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<CallableContext> input) {}
//CHECK: ^^^^^^ interface requirement mismatch
//CHECK: ^^^^^^ member 'invoke' does not match interface requirement.
//CHECK: see interface requirement declaration of 'invoke'
}
