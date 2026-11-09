// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.pipeline_rt;
import "../support/ray_context_support.slang";
import slang.runtime.core.simple_assert;
import slang.runtime.resource.simple_texture;

// Different SBT records use the same shader with distinct retained texture descriptors.
[host]
[noinline]
void registerTexturedHit(DemoRayContext context, uint geometrySlot, float value)
{
    let texture = allocateReadOnlyTexture2D<float, TextureFormat.r32Float>(1, 1);
    texture.clear(value);
    let handle = texture.getDescriptorHandle();
    context.onHit<PrimaryRay>(
        geometrySlot,
        (PrimaryRay payload, BuiltInTriangleIntersectionAttributes attributes) =>
            PrimaryRay(payload.value + uint(nonuniform(handle).Load(int3(0)))));
}

[host]
void main()
{
    let scene = makeRayTypeScene();
    DemoRayContext context = {};
    registerRayTypeCallbacks(context, 10);
    for (uint geometry = 0; geometry < 4; ++geometry)
        registerTexturedHit(context, geometry, float(100 + geometry * 17));
    let first = dispatchRayTypes(context, scene);

    // Drop the first four graph textures and allocate replacements before waiting for the GPU.
    // Native descriptor views must stay alive for the already queued dispatch.
    for (uint geometry = 0; geometry < 4; ++geometry)
        registerTexturedHit(context, geometry, float(200 + geometry * 19));
    let second = dispatchRayTypes(context, scene);
    for (uint geometry = 0; geometry < 4; ++geometry)
    {
        assert(first.readBack(geometry * 3) == 103 + geometry * 17);
        assert(second.readBack(geometry * 3) == 203 + geometry * 19);
    }
    assert(first.readBack(12) == 1013);
    assert(second.readBack(12) == 1013);
}
