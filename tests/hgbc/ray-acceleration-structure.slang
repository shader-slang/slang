//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-assembly -entry generate -profile sm_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry generate -profile sm_6_6

// BLAS geometry and scene instance lists are host data. Only an owning bindless TLAS view crosses
// into the shader; the device conversion continues to use the ordinary descriptor-handle path.
import slang.pipeline_rt;

typealias RTTypes = HWRaytracingContext<>;

[host]
DescriptorHandle<RaytracingAccelerationStructure> main(
    StructuredBuffer<float3> vertices,
    StructuredBuffer<uint> indices)
{
    RTTypes.TriangleGeometry geometry;
    geometry.vertices = vertices;
    geometry.indices = indices;
    geometry.vertexCount = 3;
    geometry.indexCount = 3;
    let geometries = RuntimeArray<RTTypes.TriangleGeometry>::alloc(2);
    geometries[0] = geometry;
    geometries[1] = geometry;
    let blas = RTTypes.BottomLevelAccelerationStructure::alloc();
    blas.build(geometries);

    RTTypes.Instance instance;
    instance.blas = blas;
    let instances = RuntimeArray<RTTypes.Instance>::alloc(2);
    instances[0] = instance;
    instance.instanceID = 1;
    instance.transform[0][3] = 2;
    instances[1] = instance;
    let scene = RaytracingAccelerationStructure::alloc();
    scene.build(instances);
    return scene.getDescriptorHandle();
}

struct Payload
{
    uint value;
}

[shader("raygeneration")]
void generate(
    uniform DescriptorHandle<RaytracingAccelerationStructure> scene,
    uniform DescriptorHandle<RWStructuredBuffer<uint>> output)
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 10;
    Payload payload = { 0 };
    TraceRay(RaytracingAccelerationStructure(scene), 0, 255, 0, 1, 0, ray, payload);
    RWStructuredBuffer<uint>(output)[0] = payload.value;
}

// CHECK-DAG: OpRaytracingBottomLevelAccelerationStructureType
// CHECK-DAG: OpRaytracingAccelerationStructureType
// CHECK-DAG: OpAllocAccelerationStructure BottomLevel
// CHECK-DAG: OpAllocAccelerationStructure TopLevel
// CHECK-DAG: OpBuildAccelerationStructure BottomLevel Triangles
// CHECK-DAG: OpBuildAccelerationStructure TopLevel Instances
// CHECK-DAG: OpDescriptorHandleType
// CHECK-DAG: OpResourceToDescriptorHandle
// SPIRV: OpTypeAccelerationStructureKHR
// SPIRV: OpConvertUToAccelerationStructureKHR
// SPIRV: OpTraceRayKHR
// DXIL: @dx.op.createHandleFromHeap
// DXIL: @dx.op.traceRay
