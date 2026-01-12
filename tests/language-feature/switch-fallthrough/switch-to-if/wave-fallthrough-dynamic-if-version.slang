// wave-fallthrough-dynamic-if-version.slang
//
// Hand-written if-version with DYNAMIC predicates that can't be optimized away.
// This tests whether the do-while(false) + if pattern works when the optimizer
// can't eliminate the control flow.

// CPU doesn't support wave ops; need GPU backend
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj

// Disabled - testing structure only
//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=value
RWStructuredBuffer<int> value;

[numthreads(4, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    uint sum = 0;

    // Read selector value
    int selector = value[threadID.x];

    // === TRANSFORMED SWITCH with dynamic predicates ===
    // The predicates are computed from input, so optimizer can't eliminate them.

    // do-while(false) wrapper for break semantics
    do {
        // Pre-compute predicates (inside the loop body)
        bool isCase0 = (selector == 0);
        // Make reachesDefault dynamic (can't be optimized to constant)
        bool reachesDefault = isCase0 | (selector != 0);  // Always true but compiler doesn't know

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
