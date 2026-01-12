// wave-done-flag-if-version.slang
//
// Hand-written if-version using a "done" flag instead of break.
// This avoids the SPIRV structured control flow issues with do-while + break.

// CPU doesn't support wave ops; need GPU backend
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -profile cs_6_0 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -compute -shaderobj

// EXPECTED OUTPUT (same as fallthrough version):
// Thread 0 (isCase0=true): WaveActiveSum(1)=1, then WaveActiveSum(10)=40 → 1+40=41 (0x29)
// Threads 1,2,3 (isCase0=false): WaveActiveSum(10)=40 (0x28)

// CHECK: 29
// CHECK-NEXT: 28
// CHECK-NEXT: 28
// CHECK-NEXT: 28

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=value
RWStructuredBuffer<int> value;

[numthreads(4, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    uint sum = 0;

    // Read selector value
    int selector = value[threadID.x];

    // === TRANSFORMED SWITCH using done flag instead of break ===
    // This pattern maintains structured control flow.
    //
    // Original:
    //   switch (selector) {
    //     case 0: sum += WaveActiveSum(1); // fallthrough
    //     default: sum += WaveActiveSum(10); break;
    //   }

    // Pre-compute predicates
    bool isCase0 = (selector == 0);
    bool reachesDefault = isCase0 | (selector != 0);  // Dynamic - can't be optimized away

    // Done flag replaces break
    bool done = false;

    // case 0: wave op then fallthrough (no done=true since it falls through)
    if (!done && isCase0)
    {
        sum += WaveActiveSum(1);
        // fallthrough - don't set done
    }

    // default: reached by case0 fallthrough OR direct match
    if (!done && reachesDefault)
    {
        sum += WaveActiveSum(10);
        done = true;  // Instead of break
    }

    value[threadID.x] = int(sum);
}
