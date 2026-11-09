// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.pipeline_rt;
import "../support/ray_context_support.slang";
import slang.runtime.core.simple_assert;

// Every invocation registers the same closest-hit specialization. Its buffer is local to this
// helper, so only the registration's captured descriptor handle keeps it alive after returning.
[host]
[noinline]
void registerBufferedHit(DemoRayContext context, uint geometrySlot, uint value)
{
    let source = RuntimeArray<uint>::alloc(1);
    source[0] = value;
    let buffer = StructuredBuffer<uint>::alloc(1);
    copy(buffer, 0, source, 0, 1);
    let handle = buffer.getDescriptorHandle();
    context.onHit<PrimaryRay>(
        geometrySlot,
        (PrimaryRay payload, BuiltInTriangleIntersectionAttributes attributes) =>
            PrimaryRay(payload.value + nonuniform(handle)[0]));
}

[host]
void main()
{
    let scene = makeRayTypeScene();
    DemoRayContext context = {};
    registerRayTypeCallbacks(context, 10);
    for (uint geometry = 0; geometry < 4; ++geometry)
        registerBufferedHit(context, geometry, 100 + geometry * 17);
    let output = dispatchRayTypes(context, scene);
    for (uint geometry = 0; geometry < 4; ++geometry)
    {
        assert(output.readBack(geometry * 3) == 103 + geometry * 17);
        assert(output.readBack(geometry * 3 + 1) == 115 + geometry * 10);
    }
    assert(output.readBack(12) == 1013);
}
