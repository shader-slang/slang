// wave-break-if-version.slang
//
// Hand-written if-version showing case 0 BREAKING (not falling through).
// Compare with wave-fallthrough-gh6441-if-version.slang to see the difference.

// CPU doesn't support wave ops; need GPU backend
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -profile cs_6_0 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -compute -shaderobj

// Thread 0 (isCase0=true): WaveActiveSum(1) = 1, then BREAK (skip default)
// Threads 1,2,3 (isCase0=false): WaveActiveSum(10) = 40

// CHECK: 1
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

    // === TRANSFORMED SWITCH (case 0 breaks, not fallthrough) ===
    // Original:
    //   switch (selector) {
    //     case 0: sum += WaveActiveSum(1); break;  // <-- BREAK!
    //     default: sum += WaveActiveSum(10); break;
    //   }

    // do-while(false) wrapper for break semantics
    do {
        // Pre-compute predicates (inside the loop body)
        bool isCase0 = (selector == 0);
        bool isDefault = !isCase0;  // Only reached if NOT case 0

        // case 0: wave op then BREAK
        if (isCase0)
        {
            sum += WaveActiveSum(1);
            break;  // exit the do-while
        }

        // default: only reached if case 0 didn't match
        if (isDefault)
        {
            sum += WaveActiveSum(10);
            break;  // exit the do-while
        }
    } while (false);

    value[threadID.x] = int(sum);
}
