// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature

import slang.neural;

typealias LongMaskConverter = NetworkParameterLayoutConverter<
    float,
    0x200000001ull,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1>;

//TEST_INPUT: set portable = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4)
uniform RWStructuredBuffer<float>.Handle portable;

//TEST_INPUT: set optimal = ubuffer(stride=4, count=272)
uniform RWStructuredBuffer<float>.Handle optimal;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=resultBuffer
RWStructuredBuffer<uint> resultBuffer;

void toOptimalLayout()
{
    let portableAddress = BindlessAddress<float>(portable);
    let optimalAddress = BindlessAddress<float>(optimal);

    NetworkParameterLayoutConverter<float, 1, 4, 2>.toOptimalLayout(portableAddress, optimalAddress);
    AllMemoryBarrierWithGroupSync();
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    toOptimalLayout();
    AllMemoryBarrierWithGroupSync();

    bool commonOk =
        optimal[0] == 1.0 &&
        optimal[256] == 9.0 &&
        optimal[257] == 10.0 &&
        optimal[258] == 0.0;

    bool targetOk = false;
    __target_switch
    {
    case cuda:
        targetOk = optimal[1] == 2.0 && optimal[8] == 3.0 && optimal[9] == 4.0 &&
            optimal[32] == 5.0 && optimal[33] == 6.0 && optimal[40] == 7.0 && optimal[41] == 8.0;
    case spirv:
        targetOk = optimal[1] == 2.0 && optimal[2] == 3.0 && optimal[3] == 4.0 &&
            optimal[16] == 5.0 && optimal[17] == 6.0 && optimal[18] == 7.0 && optimal[19] == 8.0;
    case metal:
        targetOk = optimal[1] == 2.0 && optimal[2] == 3.0 && optimal[3] == 4.0 &&
            optimal[16] == 5.0 && optimal[17] == 6.0 && optimal[18] == 7.0 && optimal[19] == 8.0;
    }

    resultBuffer[0] = commonOk ? 1 : 0;
    resultBuffer[1] = targetOk ? 1 : 0;
    resultBuffer[2] = (NetworkParameterLayoutConverter<float, 1, 4, 2>.ElementCountPhysical == 272) ? 1 : 0;
    resultBuffer[3] = (LongMaskConverter.ElementCountLogical == 36 &&
        LongMaskConverter.ElementCountPhysical == 9248) ? 1 : 0;

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
