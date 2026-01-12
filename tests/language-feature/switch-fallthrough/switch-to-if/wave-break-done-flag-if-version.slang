// wave-break-done-flag-if-version.slang
//
// Hand-written if-version using a "done" flag for BREAK case.
// Case 0 breaks (sets done=true), so default should NOT run for thread 0.

// CPU doesn't support wave ops; need GPU backend
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -capability cuda_sm_7_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -profile cs_6_0 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -compute -shaderobj

// EXPECTED OUTPUT:
// Thread 0 (isCase0=true): WaveActiveSum(1)=1, done=true, skip default → 1 (0x1)
// Threads 1,2,3 (isCase0=false): skip case0, WaveActiveSum(10)=40 (0x28)

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

    // === TRANSFORMED SWITCH using done flag for break ===
    //
    // Original:
    //   switch (selector) {
    //     case 0: sum += WaveActiveSum(1); break;  // BREAK!
    //     default: sum += WaveActiveSum(10); break;
    //   }

    // Pre-compute predicates
    bool isCase0 = (selector == 0);
    bool isDefault = !isCase0;  // Only default if not case 0

    // Done flag replaces break
    bool done = false;

    // case 0: wave op then BREAK (set done=true)
    if (!done && isCase0)
    {
        sum += WaveActiveSum(1);
        done = true;  // BREAK - skip remaining cases
    }

    // default: only run if NOT done
    if (!done && isDefault)
    {
        sum += WaveActiveSum(10);
        done = true;
    }

    value[threadID.x] = int(sum);
}
