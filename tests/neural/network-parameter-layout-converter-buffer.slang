// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -entry computeMain -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -entry computeMain -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -entry computeMain -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=ROUNDTRIP):-vk -compute -entry roundTripMain -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=ROUNDTRIP):-mtl -compute -entry roundTripMain -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=ROUNDTRIP):-cuda -compute -entry roundTripMain -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=LONGMASK):-vk -compute -entry longMaskMain -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=LONGMASK):-mtl -compute -entry longMaskMain -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=LONGMASK):-cuda -compute -entry longMaskMain -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=MULTITILE):-vk -compute -entry multiTileMain -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=MULTITILE):-mtl -compute -entry multiTileMain -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=MULTITILE):-cuda -compute -entry multiTileMain -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=HALFROUNDTRIP):-vk -compute -entry halfRoundTripMain -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=HALFROUNDTRIP):-cuda -compute -entry halfRoundTripMain -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=TOPBIT):-vk -compute -entry topBitMaskMain -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=TOPBIT):-mtl -compute -entry topBitMaskMain -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=TOPBIT):-cuda -compute -entry topBitMaskMain -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=MIXEDROUNDTRIP):-vk -compute -entry mixedBiasRoundTripMain -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=MIXEDROUNDTRIP):-cuda -compute -entry mixedBiasRoundTripMain -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature

import slang.neural;

typealias BiasConverter = NetworkParameterLayoutConverter<float, 1, 4, 2>;
typealias NoBiasConverter = NetworkParameterLayoutConverter<float, 0, 4, 2>;

typealias MultiTileConverter = NetworkParameterLayoutConverter<float, 1, 17, 33>;
typealias HalfConverter = NetworkParameterLayoutConverter<half, 1, 4, 2>;

typealias MixedBiasConverter = NetworkParameterLayoutConverter<float, 0b1010ull, 4, 5, 3, 2, 6>;

typealias ZeroMaskConverter = NetworkParameterLayoutConverter<float, 0ull, 1, 1, 1, 1, 1>;
typealias AllBiasConverter = NetworkParameterLayoutConverter<float, 0xFull, 1, 1, 1, 1, 1>;
typealias TopBitMaskConverter = NetworkParameterLayoutConverter<
    float,
    0x8000000000000001ull,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1>;

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

//TEST_INPUT: set roundTrip = ubuffer(stride=4, count=10)
uniform RWStructuredBuffer<float>.Handle roundTrip;

//TEST_INPUT: set longPortable = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0 21.0 22.0 23.0 24.0 25.0 26.0 27.0 28.0 29.0 30.0 31.0 32.0 33.0 34.0 35.0 36.0], stride=4)
uniform RWStructuredBuffer<float>.Handle longPortable;

//TEST_INPUT: set longOptimal = ubuffer(stride=4, count=9248)
uniform RWStructuredBuffer<float>.Handle longOptimal;

//TEST_INPUT: set multiPortable = ubuffer(stride=4, count=594)
uniform RWStructuredBuffer<float>.Handle multiPortable;

//TEST_INPUT: set multiOptimal = ubuffer(stride=4, count=1584)
uniform RWStructuredBuffer<float>.Handle multiOptimal;

//TEST_INPUT: set halfPortable = ubuffer(data=[1.0h 2.0h 3.0h 4.0h 5.0h 6.0h 7.0h 8.0h 9.0h 10.0h], stride=2)
uniform RWStructuredBuffer<half>.Handle halfPortable;

//TEST_INPUT: set halfOptimal = ubuffer(stride=2, count=272)
uniform RWStructuredBuffer<half>.Handle halfOptimal;

//TEST_INPUT: set halfRoundTrip = ubuffer(stride=2, count=10)
uniform RWStructuredBuffer<half>.Handle halfRoundTrip;

//TEST_INPUT: set topPortable = ubuffer(stride=4, count=66)
uniform RWStructuredBuffer<float>.Handle topPortable;

//TEST_INPUT: set topOptimal = ubuffer(stride=4, count=17408)
uniform RWStructuredBuffer<float>.Handle topOptimal;

//TEST_INPUT: set mixedPortable = ubuffer(stride=4, count=62)
uniform RWStructuredBuffer<float>.Handle mixedPortable;

//TEST_INPUT: set mixedOptimal = ubuffer(stride=4, count=1088)
uniform RWStructuredBuffer<float>.Handle mixedOptimal;

//TEST_INPUT: set mixedRoundTrip = ubuffer(stride=4, count=62)
uniform RWStructuredBuffer<float>.Handle mixedRoundTrip;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=resultBuffer
RWStructuredBuffer<uint> resultBuffer;

void toOptimalLayout(uint threadIndex, uint threadCount)
{
    let portableAddress = BindlessAddress<float>(portable);
    let optimalAddress = BindlessAddress<float>(optimal);

    BiasConverter.toOptimalLayout(portableAddress, optimalAddress, threadIndex, threadCount);
    AllMemoryBarrierWithGroupSync();
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint tid : SV_GroupIndex)
{
    toOptimalLayout(tid, 32);
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
    resultBuffer[2] = (BiasConverter.ElementCountPhysical == 272) ? 1 : 0;
    resultBuffer[3] = (LongMaskConverter.ElementCountLogical == 36 &&
        LongMaskConverter.ElementCountPhysical == 9248) ? 1 : 0;
    resultBuffer[4] = (MultiTileConverter.ElementCountLogical == 594 &&
        MultiTileConverter.ElementCountPhysical == 1584) ? 1 : 0;
    resultBuffer[5] = (HalfConverter.BytesLogical == 20 &&
        HalfConverter.BytesPhysical == 544) ? 1 : 0;
    resultBuffer[6] = (ZeroMaskConverter.ElementCountLogical == 4 &&
        ZeroMaskConverter.ElementCountPhysical == 1088) ? 1 : 0;
    resultBuffer[7] = (AllBiasConverter.ElementCountLogical == 8 &&
        AllBiasConverter.ElementCountPhysical == 1088) ? 1 : 0;
    resultBuffer[8] = (TopBitMaskConverter.ElementCountLogical == 66 &&
        TopBitMaskConverter.ElementCountPhysical == 17408) ? 1 : 0;

    // MixedBiasConverter covers four layers with bias on layers 1 and 3.
    resultBuffer[0] = resultBuffer[0] != 0 &&
        MixedBiasConverter.ElementCountLogical == 62 &&
        MixedBiasConverter.ElementCountPhysical == 1088 ? 1 : 0;

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}

void clearOptimal(uint tid)
{
    for (uint i = tid; i < 272; i += 32)
        optimal[i] = 0.0;
}

void fillOptimalWeights(uint tid)
{
    clearOptimal(tid);
    AllMemoryBarrierWithGroupSync();

    if (tid != 0)
        return;

    optimal[0] = 1.0;
    __target_switch
    {
    case cuda:
        optimal[1] = 2.0;
        optimal[8] = 3.0;
        optimal[9] = 4.0;
        optimal[32] = 5.0;
        optimal[33] = 6.0;
        optimal[40] = 7.0;
        optimal[41] = 8.0;
    case spirv:
    case metal:
        optimal[1] = 2.0;
        optimal[2] = 3.0;
        optimal[3] = 4.0;
        optimal[16] = 5.0;
        optimal[17] = 6.0;
        optimal[18] = 7.0;
        optimal[19] = 8.0;
    }
}

[shader("compute")]
[numthreads(32, 1, 1)]
void roundTripMain(uint tid : SV_GroupIndex)
{
    let optimalAddress = BindlessAddress<float>(optimal);
    let roundTripAddress = BindlessAddress<float>(roundTrip);

    fillOptimalWeights(tid);
    if (tid == 0)
    {
        optimal[256] = 9.0;
        optimal[257] = 10.0;
    }
    AllMemoryBarrierWithGroupSync();

    BiasConverter.toPortableLayout(optimalAddress, roundTripAddress, tid, 32);
    AllMemoryBarrierWithGroupSync();

    bool biasRoundTripOk =
        roundTrip[0] == 1.0 && roundTrip[1] == 2.0 && roundTrip[2] == 3.0 &&
        roundTrip[3] == 4.0 && roundTrip[4] == 5.0 && roundTrip[5] == 6.0 &&
        roundTrip[6] == 7.0 && roundTrip[7] == 8.0 && roundTrip[8] == 9.0 &&
        roundTrip[9] == 10.0;

    // A no-bias layer has only the first eight logical elements. The final two slots
    // should keep the previous bias round-trip values, proving toPortableLayout did
    // not read or write the reserved optimal-layout bias slot.
    fillOptimalWeights(tid);
    AllMemoryBarrierWithGroupSync();
    NoBiasConverter.toPortableLayout(optimalAddress, roundTripAddress, tid, 32);
    AllMemoryBarrierWithGroupSync();

    bool noBiasRoundTripOk =
        roundTrip[0] == 1.0 && roundTrip[1] == 2.0 && roundTrip[2] == 3.0 &&
        roundTrip[3] == 4.0 && roundTrip[4] == 5.0 && roundTrip[5] == 6.0 &&
        roundTrip[6] == 7.0 && roundTrip[7] == 8.0 && roundTrip[8] == 9.0 &&
        roundTrip[9] == 10.0;

    resultBuffer[0] = biasRoundTripOk ? 1 : 0;
    resultBuffer[1] = noBiasRoundTripOk ? 1 : 0;
    resultBuffer[2] = (BiasConverter.ElementCountLogical == 10) ? 1 : 0;
    resultBuffer[3] = (NoBiasConverter.ElementCountLogical == 8) ? 1 : 0;

    // ROUNDTRIP: 1
    // ROUNDTRIP-NEXT: 1
    // ROUNDTRIP-NEXT: 1
    // ROUNDTRIP-NEXT: 1
}


[shader("compute")]
[numthreads(32, 1, 1)]
void longMaskMain(uint tid : SV_GroupIndex)
{
    let portableAddress = BindlessAddress<float>(longPortable);
    let optimalAddress = BindlessAddress<float>(longOptimal);

    LongMaskConverter.toOptimalLayout(portableAddress, optimalAddress, tid, 32);
    AllMemoryBarrierWithGroupSync();

    bool longMaskOk =
        longOptimal[0] == 1.0 &&
        longOptimal[256] == 2.0 &&
        longOptimal[33 * 272] == 35.0 &&
        longOptimal[33 * 272 + 256] == 36.0;

    resultBuffer[0] = longMaskOk ? 1 : 0;

    // LONGMASK: 1
}

[shader("compute")]
[numthreads(64, 1, 1)]
void multiTileMain(uint tid : SV_GroupIndex)
{
    for (uint i = tid; i < uint(MultiTileConverter.ElementCountLogical); i += 64)
        multiPortable[i] = float(i + 1);
    for (uint i = tid; i < uint(MultiTileConverter.ElementCountPhysical); i += 64)
        multiOptimal[i] = 0.0;
    AllMemoryBarrierWithGroupSync();

    let portableAddress = BindlessAddress<float>(multiPortable);
    let optimalAddress = BindlessAddress<float>(multiOptimal);

    MultiTileConverter.toOptimalLayout(portableAddress, optimalAddress, tid, 64);
    AllMemoryBarrierWithGroupSync();

    if (tid == 0)
    {
        bool weightsOk = false;
        __target_switch
        {
        case cuda:
            weightsOk =
                multiOptimal[0] == 1.0 &&
                multiOptimal[768] == 17.0 &&
                multiOptimal[512] == 545.0 &&
                multiOptimal[1280] == 561.0;
        case spirv:
            weightsOk =
                multiOptimal[0] == 1.0 &&
                multiOptimal[256] == 17.0 &&
                multiOptimal[1024] == 545.0 &&
                multiOptimal[1280] == 561.0;
        case metal:
            weightsOk =
                multiOptimal[0] == 1.0 &&
                multiOptimal[16] == 17.0 &&
                multiOptimal[1024] == 545.0 &&
                multiOptimal[1040] == 561.0;
        }

        bool biasOk =
            multiOptimal[1536] == 562.0 &&
            multiOptimal[1568] == 594.0 &&
            multiOptimal[1569] == 0.0;
        bool sizeOk = MultiTileConverter.ElementCountLogical == 594 &&
            MultiTileConverter.ElementCountPhysical == 1584;
        resultBuffer[0] = (weightsOk && biasOk) ? 1 : 0;
        resultBuffer[1] = sizeOk ? 1 : 0;
    }

    // MULTITILE: 1
    // MULTITILE-NEXT: 1
}

[shader("compute")]
[numthreads(32, 1, 1)]
void halfRoundTripMain(uint tid : SV_GroupIndex)
{
    for (uint i = tid; i < uint(HalfConverter.ElementCountPhysical); i += 32)
        halfOptimal[i] = half(0.0);
    for (uint i = tid; i < uint(HalfConverter.ElementCountLogical); i += 32)
        halfRoundTrip[i] = half(0.0);
    AllMemoryBarrierWithGroupSync();

    let portableAddress = BindlessAddress<half>(halfPortable);
    let optimalAddress = BindlessAddress<half>(halfOptimal);
    let roundTripAddress = BindlessAddress<half>(halfRoundTrip);

    HalfConverter.toOptimalLayout(portableAddress, optimalAddress, tid, 32);
    AllMemoryBarrierWithGroupSync();
    HalfConverter.toPortableLayout(optimalAddress, roundTripAddress, tid, 32);
    AllMemoryBarrierWithGroupSync();

    if (tid == 0)
    {
        bool roundTripOk = true;
        for (int i = 0; i < HalfConverter.ElementCountLogical; ++i)
            roundTripOk = roundTripOk && halfRoundTrip[i] == half(i + 1);
        resultBuffer[0] = roundTripOk ? 1 : 0;
    }

    // HALFROUNDTRIP: 1
}

[shader("compute")]
[numthreads(64, 1, 1)]
void topBitMaskMain(uint tid : SV_GroupIndex)
{
    for (uint i = tid; i < uint(TopBitMaskConverter.ElementCountLogical); i += 64)
        topPortable[i] = 0.0;
    for (uint i = tid; i < uint(TopBitMaskConverter.ElementCountPhysical); i += 64)
        topOptimal[i] = -1.0;
    AllMemoryBarrierWithGroupSync();

    if (tid == 0)
    {
        topPortable[0] = 101.0;
        topPortable[1] = 102.0;
        topPortable[2] = 201.0;
        topPortable[64] = 6401.0;
        topPortable[65] = 6402.0;
    }
    AllMemoryBarrierWithGroupSync();

    let portableAddress = BindlessAddress<float>(topPortable);
    let optimalAddress = BindlessAddress<float>(topOptimal);
    TopBitMaskConverter.toOptimalLayout(portableAddress, optimalAddress, tid, 64);
    AllMemoryBarrierWithGroupSync();

    if (tid == 0)
    {
        static const int LayerPhysicalCount = 272;
        bool topBitOk =
            topOptimal[0] == 101.0 &&
            topOptimal[256] == 102.0 &&
            topOptimal[LayerPhysicalCount] == 201.0 &&
            topOptimal[LayerPhysicalCount + 256] == 0.0 &&
            topOptimal[62 * LayerPhysicalCount + 256] == 0.0 &&
            topOptimal[63 * LayerPhysicalCount] == 6401.0 &&
            topOptimal[63 * LayerPhysicalCount + 256] == 6402.0;
        resultBuffer[0] = topBitOk ? 1 : 0;
    }

    // TOPBIT: 1
}

[shader("compute")]
[numthreads(64, 1, 1)]
void mixedBiasRoundTripMain(uint tid : SV_GroupIndex)
{
    for (uint i = tid; i < uint(MixedBiasConverter.ElementCountLogical); i += 64)
    {
        mixedPortable[i] = float(i + 1);
        mixedRoundTrip[i] = 0.0;
    }
    for (uint i = tid; i < uint(MixedBiasConverter.ElementCountPhysical); i += 64)
        mixedOptimal[i] = -1.0;
    AllMemoryBarrierWithGroupSync();

    let portableAddress = BindlessAddress<float>(mixedPortable);
    let optimalAddress = BindlessAddress<float>(mixedOptimal);
    let roundTripAddress = BindlessAddress<float>(mixedRoundTrip);

    MixedBiasConverter.toOptimalLayout(portableAddress, optimalAddress, tid, 64);
    AllMemoryBarrierWithGroupSync();

    MixedBiasConverter.toPortableLayout(optimalAddress, roundTripAddress, tid, 64);
    AllMemoryBarrierWithGroupSync();

    if (tid == 0)
    {
        static const int LayerPhysicalCount = 272;
        bool biasSlotsOk =
            mixedOptimal[256] == 0.0 &&
            mixedOptimal[LayerPhysicalCount + 256] == 36.0 &&
            mixedOptimal[LayerPhysicalCount + 258] == 38.0 &&
            mixedOptimal[2 * LayerPhysicalCount + 256] == 0.0 &&
            mixedOptimal[3 * LayerPhysicalCount + 256] == 57.0 &&
            mixedOptimal[3 * LayerPhysicalCount + 261] == 62.0;

        bool roundTripOk = true;
        for (int i = 0; i < MixedBiasConverter.ElementCountLogical; ++i)
            roundTripOk = roundTripOk && mixedRoundTrip[i] == float(i + 1);

        resultBuffer[0] = biasSlotsOk ? 1 : 0;
        resultBuffer[1] = roundTripOk ? 1 : 0;
    }

    // MIXEDROUNDTRIP: 1
    // MIXEDROUNDTRIP-NEXT: 1
}
