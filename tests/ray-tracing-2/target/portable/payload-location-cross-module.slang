//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -I tests/ray-tracing-2/target/portable -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly

// The schema and its two same-shape generic payloads live in another module. Linking must resolve
// both uses through their canonical semantic identities and keep their program-local Vulkan
// locations distinct.

import slang.raytracing;
import payload_location_cross_module_support;

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    PayloadValue<FirstTag> first = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, first);

    PayloadValue<SecondTag> second = {};
    tracer.trace(desc, scene, program, second);

    // An alias keeps the canonical identity of its target and therefore reuses the first
    // payload's location instead of creating a third partition.
    FirstPayloadAlias firstAlias = {};
    tracer.trace(desc, scene, program, firstAlias);
}

// SPIRV-DAG: OpVariable {{.*}} RayPayloadKHR {{.*}}Location 0
// SPIRV-DAG: OpVariable {{.*}} RayPayloadKHR {{.*}}Location 1
// SPIRV-DAG: OpVariable {{.*}} IncomingRayPayloadKHR {{.*}}Location 0
// SPIRV-DAG: OpVariable {{.*}} IncomingRayPayloadKHR {{.*}}Location 1
