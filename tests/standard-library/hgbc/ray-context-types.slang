// TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

import slang.pipeline_rt;

struct Primary : IRayType { uint value; }
struct Shadow : IRayType { bool visible; }

typealias Types = HWRaytracingContext<>;
typealias Narrow = HWRaytracingContext<Primary>;
typealias Wide = HWRaytracingContext<Primary, Shadow>;

// Geometry does not acquire the context's payload pack. The empty specialization is a
// type namespace; constructing an empty context is still rejected by ray-context-invalid-pack.
[host]
uint main()
{
    static_assert(__type_equals<Narrow.AccelerationStructure, Wide.AccelerationStructure>(), "TLAS identity");
    static_assert(__type_equals<Types.BottomLevelAccelerationStructure, Wide.BottomLevelAccelerationStructure>(), "BLAS identity");
    static_assert(__type_equals<Narrow.TriangleGeometry, Wide.TriangleGeometry>(), "triangle identity");
    static_assert(__type_equals<Narrow.TriangleGeometryDesc<RWStructuredBuffer<float3>, StructuredBuffer<uint>>,
        Wide.TriangleGeometryDesc<RWStructuredBuffer<float3>, StructuredBuffer<uint>>>(), "writable geometry identity");
    static_assert(__type_equals<Types.AABB, Wide.AABB>(), "bounds identity");
    static_assert(__type_equals<Narrow.AABBGeometry, Wide.AABBGeometry>(), "procedural geometry identity");
    static_assert(__type_equals<Narrow.Instance, Wide.Instance>(), "instance identity");
    static_assert(__type_equals<Types.AccelerationStructure.Handle, DescriptorHandle<RaytracingAccelerationStructure>>(), "native handle identity");

    let blas = Types.BottomLevelAccelerationStructure::alloc();
    Wide.Instance instance = Narrow.Instance(blas);
    instance.instanceID = 7;
    let instances = RuntimeArray<Types.Instance>::alloc(1);
    instances[0] = instance;
    let scene = Narrow.AccelerationStructure::alloc();
    scene.build(instances);
    Wide.AccelerationStructure.Handle handle = scene.getDescriptorHandle();
    return instance.instanceID;
}

// CHECK: OpRaytracingBottomLevelAccelerationStructureType
// CHECK: OpRaytracingAccelerationStructureType
// CHECK: OpBuildAccelerationStructure TopLevel Instances
