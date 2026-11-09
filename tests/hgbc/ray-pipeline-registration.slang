//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-assembly -entry generate -profile sm_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry generate -profile sm_6_6

// Registration retains the specialized entry point separately from its runtime argument values.
// Empty arguments must not renumber the remaining reflected shader parameters.
import slang.pipeline_rt;

[raypayload]
struct Payload
{
    uint value : read(caller, closesthit, miss)
        : write(caller, closesthit, miss);
}

struct Empty
{
}

[shader("miss")]
void miss<let bias : uint>(inout Payload payload, uniform Empty empty, uniform uint value)
{
    payload.value = value + bias;
}

[shader("closesthit")]
void hit(
    inout Payload payload,
    in BuiltInTriangleIntersectionAttributes attributes,
    uniform uint value)
{
    payload.value = value + uint(attributes.barycentrics.x);
}

[shader("raygeneration")]
void generate(uniform RaytracingPipeline pipeline, uniform RWStructuredBuffer<uint> output)
{
    output[DispatchRaysIndex().x] = getBindingOffset(pipeline);
}

[host]
void launch<let depth : uint>(RaytracingPipeline pipeline, RWStructuredBuffer<uint> output)
{
    __dispatchRays(generate, uint3(1, 1, 1), depth)(pipeline, output);
}

[host]
void main(RWStructuredBuffer<uint> output)
{
    let pipeline = makeRaytracingPipeline(2);
    __setRayMiss(pipeline, 0, miss<1>)(Empty(), 7);
    __setRayMiss(pipeline, 1, miss<2>)(Empty(), 9);
    for (uint geometry = 0; geometry < 2; ++geometry)
        __setRayHitShader(pipeline, geometry, 0, hit)(geometry + 3);
    launch<2>(pipeline, output);
}

// CHECK-DAG: OpRaytracingPipelineType
// CHECK-DAG: OpMakeRaytracingPipeline
// CHECK-DAG: OpSpecializeSlangEntryPoint
// CHECK-DAG: OpSetRayMiss {{.*}} 2 {{.*}}
// CHECK-DAG: OpRayHitGroup Triangles {{.*}} ClosestHit
// CHECK-DAG: OpSetRayHitGroup {{.*}} ClosestHit 2 {{.*}}
// CHECK-DAG: OpDispatchRays {{.*}} 2 {{.*}}

// The device projection must read the reflected record field instead of folding it to zero.
// SPIRV: OpMemberName %{{.*}} 0 "bindingOffset"
// SPIRV: %[[OFFSET_PTR:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_ShaderRecordBuffer{{KHR|NV}}_uint
// SPIRV: %[[OFFSET:[A-Za-z0-9_]+]] = OpLoad %uint %[[OFFSET_PTR]]
// SPIRV: {{OpStore %[A-Za-z0-9_]+|OpReturnValue}} %[[OFFSET]]
// DXIL: uint bindingOffset
// DXIL: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
// DXIL: call void @dx.op.rawBufferStore.i32
