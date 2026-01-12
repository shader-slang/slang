// wave-fallthrough-gh6441-if-version.slang
//
// Hand-written if-version of wave-fallthrough-gh6441.slang
// This shows what the switch-to-if transformation SHOULD produce.
// Compare the IR of this file with the switch version to understand
// the transformation.

// CPU doesn't support wave ops; need GPU backend
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -profile cs_6_0 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -compute -shaderobj

// EXPECTED OUTPUT:
// All threads reconverge at each wave op because there's no divergent branching.
// - Thread 0 (isCase0=true): runs case0 body, then runs default body
//   WaveActiveSum(1) = 1 (only thread 0 has isCase0=true, so only 1 contributes)
//   WaveActiveSum(10) = 40 (all 4 threads have reachesDefault=true)
//   Total: 1 + 40 = 41 (0x29)
// - Threads 1,2,3 (isCase0=false): skip case0 body, run default body
//   WaveActiveSum(10) = 40
//   Total: 40 (0x28)

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

    // === TRANSFORMED SWITCH ===
    // Original:
    //   switch (selector) {
    //     case 0: sum += WaveActiveSum(1); // fallthrough
    //     default: sum += WaveActiveSum(10); break;
    //   }

    // do-while(false) wrapper for break semantics
    do {
        // Pre-compute predicates (inside the loop body)
        bool isCase0 = (selector == 0);
        bool reachesDefault = true;  // case 0 falls through OR direct default match
        // case 0: wave op then fallthrough
        if (isCase0)
        {
            sum += WaveActiveSum(1);
            // fallthrough - no break, continue to next check
        }

        // default: reached by case0 fallthrough OR direct match
        if (reachesDefault)
        {
            sum += WaveActiveSum(10);
            break;  // exit the do-while
        }
    } while (false);

    value[threadID.x] = int(sum);
}
