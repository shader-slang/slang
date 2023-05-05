//TEST(compute, vulkan):COMPARE_COMPUTE_EX():-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX():-slang -dx12 -use-dxil -compute -shaderobj -output-using-type
>>>>>>> d4f4681b5 (a)

//TEST_INPUT:ubuffer(data=[0 3 2 2], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

func saturated_cooperation<A : __BuiltinType, B, C>(cooperate : (A, B) -> C, fallback : (A, B) -> C, A input, B otherArgs) -> C
{
    const bool isWaveSaturated = WaveActiveCountBits(true) == WaveGetLaneCount();
    if(isWaveSaturated)
    {
        let lanesWithSameInput = WaveMatch(input).x;
        // Keep least significant lane in our set
        let ourRepresentative = lanesWithSameInput & -lanesWithSameInput;
        // The representative lanes for all lanes
        var allRepresentatives = WaveActiveBitOr(ourRepresentative);

        C ret;

        // Iterate over set bits in mask from low to high.
        // In each iteration the lowest bit is cleared.
        while(bool(allRepresentatives))
        {
            // Broadcast input across warp.
            let laneIdx = firstbitlow(allRepresentatives);
            let uniformInput = WaveReadLaneAt(input, int(laneIdx));

            // All lanes perform some cooperative computation with dynamic
            // uniform input
            C c = cooperate(uniformInput, otherArgs);

            // Update our return value until it
            if(bool(allRepresentatives & ourRepresentative))
                ret = c;

            // Clear the lowest bit
            allRepresentatives &= allRepresentatives - 1;
        }

        return ret;
    }
    else
    {
        return fallback(input, otherArgs);
    }
}

static int count = 0;

int cooperate(float x, int i)
{
    count += 1;
    return int(x) * 2 + i;
}

int fallback(float x, int i)
{
    count += 100;
    return int(x) * 3 + i;
}

// Make sure that we have enough invocations to saturate the first workgroup
[numthreads(128, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    // The values we're cooperating over are {0, 2, 3}
    // We track the number of sets evaluated in the "count" variable, and write
    // that at index 0
    let i = tig < 4 ? float(outputBuffer[tig]) : 0;
    // In this test we also check that the 'otherArgs' are not synchronized
    // across lanes
    let x = saturated_cooperation(cooperate, fallback, i, int(tig));
    if(tig < 4)
        outputBuffer[tig] = tig == 0 ? count : x;

}
