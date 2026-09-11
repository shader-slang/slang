//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry main -stage compute -dump-ir -no-codegen

import slang.raytracing;

// CHECK-DAG: closest_hit_stage_interface(
// CHECK-DAG: any_hit_stage_interface(
// CHECK-DAG: intersection_stage_interface(
// CHECK-DAG: miss_stage_interface(
// CHECK-DAG: callable_stage_interface(
// CHECK-DAG: let %localx5FIClosestHitShader{{.*}}= interface(

struct TestPayload {}
struct TestRecord {}
struct Attributes {}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
    typealias Record = TestRecord;
}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias Record = TestRecord;
}

struct CallableContext : rt::ICallableGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = uint;
    typealias Record = TestRecord;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input) {}
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input) {}
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;
    void invoke(rt::IntersectionInput<HitContext> input) {}
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input) {}
}

struct Callable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<CallableContext> input) {}
}

namespace local
{
interface IClosestHitShader<T>
{
    void invoke();
}
}

struct OrdinaryInterfaceImplementation : local::IClosestHitShader<int>
{
    void invoke() {}
}

void invokeOrdinaryInterface(local::IClosestHitShader<int> stage)
{
    stage.invoke();
}

[numthreads(1, 1, 1)]
void main()
{
    // Keep the user-defined interface reachable so this test observes its ordinary IR identity
    // even when dead-code elimination removes unused interface declarations.
    invokeOrdinaryInterface(OrdinaryInterfaceImplementation());
}
