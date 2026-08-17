// Verifies that explicit coverage bindings treat an unsized descriptor
// array as occupying every binding at or above its declared binding in
// the same descriptor set. Vulkan variable-descriptor-count bindings
// must remain the highest binding in their set, so pinning coverage
// after one is still a collision.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage-binding 5 0

[allow("overlapping-bindings")]
[[vk::binding(0, 0)]]
RWStructuredBuffer<uint> bindlessBuffers[];
//CHECK:                 ^^^^^^^^^^^^^^^ `-trace-coverage-binding` collides with an existing parameter

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        bindlessBuffers[0][0] = 1;
    }
}
