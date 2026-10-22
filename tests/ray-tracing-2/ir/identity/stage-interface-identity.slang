//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -dump-ir -no-codegen

import slang.raytracing;

// Stage identity is checked in the AST. All five contracts lower to ordinary IR interfaces.
// CHECK-DAG: let %rtx5FIClosestHitShader{{.*}}= interface(
// CHECK-DAG: let %rtx5FIAnyHitShader{{.*}}= interface(
// CHECK-DAG: let %rtx5FIIntersectionShader{{.*}}= interface(
// CHECK-DAG: let %rtx5FIMissShader{{.*}}= interface(
// CHECK-DAG: let %rtx5FICallableShader{{.*}}= interface(

struct TestPayload {}
struct TestRecord {}
struct Attributes {}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
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
    static void invoke(rt::ClosestHitInput<HitContext> input) {}
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    static void invoke(rt::AnyHitInput<HitContext> input) {}
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;
    static void invoke(rt::IntersectionInput<HitContext> input) {}
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    static void invoke(rt::MissInput<MissContext> input) {}
}

struct Callable : rt::ICallableShader
{
    typealias Context = CallableContext;
    static void invoke(rt::CallableInput<CallableContext> input) {}
}
