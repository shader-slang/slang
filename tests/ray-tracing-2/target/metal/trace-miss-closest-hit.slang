//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitRecord {}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = HitRecord;
}

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = 1;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct MissRecord {}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Record = MissRecord;
}

struct TestMiss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.value = 2;
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<TestMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
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
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    output[0] = payload.value;
}

// METAL: struct Schema_rayData_0
// METAL: TestPayload_0 payload_{{[0-9]+}};
// METAL: struct rt_TraceProgramDescriptorResources_default_0
// METAL: metal::visible_function_table<void(Schema_rayData_0 thread*)> missFunctions_0;
// METAL: metal::visible_function_table<void(Schema_rayData_0 thread*)> closestHitFunctions_0;
// METAL: {{\[\[kernel\]\]}} void main_0(
// METAL: thread Schema_rayData_0 [[RAY_DATA:rayData_[0-9]+]];
// METAL: metal::raytracing::intersector<metal::raytracing::instancing> _slang_intersector;
// METAL: _slang_intersector.assume_geometry_type(metal::raytracing::geometry_type::triangle);
// METAL: _slang_intersector.intersect(
// METAL-NOT: intersectionFunctions
// METAL: scene_1, desc_0.instanceMask_0);
// METAL: _S{{[0-9]+}}[desc_0.missIndex_0](&rayData_{{[0-9]+}});
// METAL: ](&rayData_{{[0-9]+}});
// METAL: {{\[\[visible\]\]}} void __slang_structural_rt_{{[0-9a-f]+}}(Schema_rayData_0 thread* [[MISS_DATA:rayData_[0-9]+]])
// METAL: (&[[MISS_DATA]]->payload_{{[0-9]+}})->value_0 = 2U;
// METAL: {{\[\[visible\]\]}} void __slang_structural_rt_{{[0-9a-f]+}}(Schema_rayData_0 thread* [[HIT_DATA:rayData_[0-9]+]])
// METAL: (&[[HIT_DATA]]->payload_{{[0-9]+}})->value_0 = 1U;
