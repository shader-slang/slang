// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.pipeline_rt;
import "../support/ray_context_support.slang";
import slang.runtime.core.simple_assert;

void checkResults(RWStructuredBuffer<uint> output, uint bias)
{
    for (uint geometrySlot = 0; geometrySlot < 4; ++geometrySlot)
    {
        assert(output.readBack(geometrySlot * 3) == 3 + bias + geometrySlot * 10);
        assert(output.readBack(geometrySlot * 3 + 1) == 105 + bias + geometrySlot * 10);
        assert(output.readBack(geometrySlot * 3 + 2) == (geometrySlot < 2 ? 7 : 19));
    }
    assert(output.readBack(12) == 1003 + bias);
    assert(output.readBack(13) == 2005 + bias);
    assert(output.readBack(14) == 99);
}

[host]
void main()
{
    let scene = makeRayTypeScene();
    DemoRayContext context = {};
    registerRayTypeCallbacks(context, 10);
    let first = dispatchRayTypes(context, scene);
    // Capture changes replace SBT data while retaining the same shader specializations.
    // Queue the second dispatch before reading either result to exercise snapshot lifetimes.
    let alias = context;
    registerRayTypeCallbacks(alias, 30);
    let second = dispatchRayTypes(context, scene);
    checkResults(first, 10);
    checkResults(second, 30);
}
