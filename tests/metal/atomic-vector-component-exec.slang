// Atomics on elements of a vector must update exactly the addressed element (#13551).

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=16):out,name=vecs
RWStructuredBuffer<uint4> vecs;

groupshared uint4 gs;

[numthreads(4, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    if (id.x == 0)
        gs = uint4(0);
    GroupMemoryBarrierWithGroupSync();

    InterlockedAdd(vecs[0].y, 1u);
    InterlockedAdd(vecs[0][id.x], id.x + 1);
    InterlockedAdd(gs[id.x & 1], 10u);
    GroupMemoryBarrierWithGroupSync();

    if (id.x == 0)
        vecs[1] = gs;
}

// Each of the 4 threads adds 1 to `vecs[0].y` and `id.x + 1` to `vecs[0][id.x]`, giving 1 6 3 4.
// Threads 0 and 2 add 10 to `gs.x`, threads 1 and 3 to `gs.y`; `gs.z` and `gs.w` stay 0.
// BUF: 1
// BUF-NEXT: 6
// BUF-NEXT: 3
// BUF-NEXT: 4
// BUF-NEXT: 20
// BUF-NEXT: 20
// BUF-NEXT: 0
// BUF-NEXT: 0
