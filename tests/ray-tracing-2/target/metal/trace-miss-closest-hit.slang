//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = HitRecord;
}

struct TestClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = 1;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct MissRecord {}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = TestTraceContext;
    typealias Record = MissRecord;
}

struct TestMiss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.value = 2;
    }
}

struct MissGroup : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = MissContext;
    typealias Miss = TestMiss;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestTraceContext, HitGroup>;
    typealias MissGroups = rt::MissGroupList<TestTraceContext, MissGroup>;
    typealias CallableGroups = rt::NoCallableGroups<TestTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc;
    desc.ray.origin = float3(0.0, 0.0, 0.0);
    desc.ray.tMin = 0.001;
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 1000.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;
    desc.sbtOffset = 0;
    desc.sbtStride = 1;
    desc.missIndex = 0;

    TestPayload payload = { 0 };
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
    output[0] = payload.value;
}

// METAL: struct ProgramLayout_rayData_0
// METAL: TestPayload_0 payload_{{[0-9]+}};
// METAL: struct rt_TraceProgramDescriptorResources_default_0
// METAL: metal::visible_function_table<void(ProgramLayout_rayData_0 thread*)> missFunctions_0;
// METAL: metal::visible_function_table<void(ProgramLayout_rayData_0 thread*)> closestHitFunctions_0;
// METAL: {{\[\[kernel\]\]}} void main_0(
// METAL: thread ProgramLayout_rayData_0 [[RAY_DATA:rayData_[0-9]+]];
// METAL: metal::raytracing::intersector<metal::raytracing::instancing> _slang_intersector;
// METAL: _slang_intersector.assume_geometry_type(metal::raytracing::geometry_type::triangle);
// METAL: _slang_intersector.intersect(
// METAL-NOT: intersectionFunctions
// METAL: scene_1, desc_0.instanceMask_0);
// METAL: _S{{[0-9]+}}[desc_0.missIndex_0](&rayData_{{[0-9]+}});
// METAL: uint _slang_hit_slot = _S{{[0-9]+}}[_S{{[0-9]+}}[0] + _slang_result.instance_id]
// METAL: _S{{[0-9]+}}[_slang_hit_slot](&rayData_{{[0-9]+}});
// METAL: {{\[\[visible\]\]}} void TestMiss_0(ProgramLayout_rayData_0 thread* [[MISS_DATA:rayData_[0-9]+]])
// METAL: (&[[MISS_DATA]]->payload_{{[0-9]+}})->value_0 = 2U;
// METAL: {{\[\[visible\]\]}} void TestClosestHit_0(ProgramLayout_rayData_0 thread* [[HIT_DATA:rayData_[0-9]+]])
// METAL: (&[[HIT_DATA]]->payload_{{[0-9]+}})->value_0 = 1U;
