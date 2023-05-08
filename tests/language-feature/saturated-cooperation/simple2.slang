//TEST(compute, vulkan):COMPARE_COMPUTE_EX():-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX():-dx12 -profile sm_6_5 -use-dxil -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 3 2 2], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

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
