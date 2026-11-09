// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.pipeline_rt;
import "../support/ray_context_support.slang";
import slang.runtime.core.simple_assert;

[host]
void main()
{
    let scene = makeRayTypeScene();
    let sceneHandle = scene.getDescriptorHandle();
    DemoRayContext context = {};
    registerRayTypeCallbacks(context, 10);
    // A closest-hit callback captures its owning tracker and launches a different payload type.
    // The secondary ray misses so the result proves that the second miss record was selected.
    context.onHit<PrimaryRay>(
        0,
        (PrimaryRay payload, BuiltInTriangleIntersectionAttributes attributes) =>
        {
            RayDesc ray;
            ray.Origin = float3(20.0, 0.0, -1.0);
            ray.Direction = float3(0.0, 0.0, 1.0);
            ray.TMin = 0.001;
            ray.TMax = 100.0;
            VisibilityRay secondary = { 5, 0 };
            context.traceRay<VisibilityRay>(*sceneHandle, ray, secondary);
            return PrimaryRay(payload.value + secondary.value);
        });
    let output = dispatchRayTypes<2>(context, scene);
    assert(output.readBack(0) == 2018);
    assert(output.readBack(1) == 115);
    assert(output.readBack(2) == 7);
    assert(output.readBack(3) == 23);
    assert(output.readBack(12) == 1013);
}
