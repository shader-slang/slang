//TEST(compute, vulkan):COMPARE_COMPUTE_EX():-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX():-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 3 2 2], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

func saturated_cooperation<A : __BuiltinType, B>(cooperate : A -> B, fallback : A -> B, A input) -> B
{
    const bool isWaveSaturated = WaveActiveCountBits(true) == WaveGetLaneCount();
    if(isWaveSaturated)
    {
        let lanesWithSameInput = WaveMatch(input).x;
        // Keep least significant lane in our set
        let ourRepresentative = lanesWithSameInput & -lanesWithSameInput;
        // The representative lanes for all lanes
        var allRepresentatives = WaveActiveBitOr(ourRepresentative);

        B ret;

        // Iterate over set bits in mask from low to high.
        // In each iteration the lowest bit is cleared.
        while(bool(allRepresentatives))
        {
            // Broadcast input across warp.
            let laneIdx = firstbitlow(allRepresentatives);
            let uniformInput = WaveReadLaneAt(input, int(laneIdx));

            // All lanes perform some cooperative computation with dynamic
            // uniform input
            B b = cooperate(uniformInput);

            // Update our return value until it
            if(bool(allRepresentatives & ourRepresentative))
                ret = b;

            // Clear the lowest bit
            allRepresentatives &= allRepresentatives - 1;
        }

        return ret;
    }
    else
    {
        return fallback(input);
    }
}

static int count = 0;

int cooperate(float x)
{
    count += 1;
    return int(x) * 2;
}

int fallback(float x)
{
    count += 100;
    return int(x) * 3;
}

// Make sure that we have enough invocations to saturate the first workgroup
[numthreads(128, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    // The values we're cooperating over are {0, 2, 3}
    // We track the number of sets evaluated in the "count" variable, and write
    // that at index 0
    let i = tig < 4 ? float(outputBuffer[tig]) : 0;
    let x = saturated_cooperation<float, int>(cooperate, fallback, i);
    if(tig < 4)
        outputBuffer[tig] = tig == 0 ? count : x;

}
