// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12
// TEST:INTERPRET(hgvm,filecheck=TRACE): -api vulkan -trace-all -color never
// TEST:INTERPRET(hgvm,filecheck=TRACE): -api d3d12 -trace-all -color never

// clang-format off
// TRACE: acceleration-structure-build batch=[[BATCH:[0-9]+]] execution=[[BLAS:[0-9]+]] acceleration-structure=[[BLAS_ADDRESS:0x[0-9a-f]+]] kind=blas
// TRACE: acceleration-structure-build batch=[[BATCH]] execution=[[TLAS:[0-9]+]] acceleration-structure=[[TLAS_ADDRESS:0x[0-9a-f]+]] kind=tlas
// TRACE: acceleration-structure-build-complete batch=[[BATCH]] execution=[[BLAS]] acceleration-structure=[[BLAS_ADDRESS]] kind=blas gpu-duration-ms={{[0-9]+\.[0-9]+}}
// TRACE: acceleration-structure-build-complete batch=[[BATCH]] execution=[[TLAS]] acceleration-structure=[[TLAS_ADDRESS]] kind=tlas gpu-duration-ms={{[0-9]+\.[0-9]+}}
// clang-format on

import slang.runtime.compute.grid;
import slang.pipeline_rt;
import "../support/ray_context_support.slang";
import slang.runtime.core.simple_assert;

[host]
void main()
{
    let scene = makeRayTypeScene();
    let sceneHandle = scene.getDescriptorHandle();
    let output = RWStructuredBuffer<uint>::alloc(5);
    let outputHandle = output.getDescriptorHandle();
    // Ordinary compute binding must realize the TLAS without a ray tracker or SBT dispatch.
    Grid(uint(5)).dispatch(
        (uint index) =>
        {
            RayDesc ray;
            ray.Origin = float3(index < 4 ? float(index * 2) - 1.0 : 20.0, 0.0, -1.0);
            ray.Direction = float3(0.0, 0.0, 1.0);
            ray.TMin = 0.001;
            ray.TMax = 100.0;
            RayQuery<RAY_FLAG_FORCE_OPAQUE> query;
            query.TraceRayInline(*sceneHandle, RAY_FLAG_NONE, 0xff, ray);
            while (query.Proceed())
            {
            }
            outputHandle[index] =
                query.CommittedStatus() == COMMITTED_TRIANGLE_HIT
                    ? query.CommittedInstanceID() * 10 + query.CommittedGeometryIndex()
                    : 999;
        });
    assert(output.readBack(0) == 70);
    assert(output.readBack(1) == 71);
    assert(output.readBack(2) == 190);
    assert(output.readBack(3) == 191);
    assert(output.readBack(4) == 999);
}
