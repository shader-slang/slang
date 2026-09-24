//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;


// Compare singleton reductions as raw bits: neither adding zero nor multiplying by one is
// equivalent to passthrough for signaling NaNs. The matrix/vector payloads differ by component.
uint64_t payload(uint selector)
{
    switch (selector % 4)
    {
    case 0: return (uint64_t(0x7FF00000u) << 32) | uint64_t(0x12345u);
    case 1: return (uint64_t(0xFFF80000u) << 32) | uint64_t(0x6789Au);
    case 2: return uint64_t(1) << 63;
    default: return uint64_t(0x7FF00000u) << 32;
    }
}

bool checkZeroPartition(uint mask, uint rank, bool callerSeed)
{
    double negativeZero = bit_cast<double>(uint64_t(1) << 63);
    uint4 partition = uint4(mask, 0, 0, 0);
    uint64_t expectedSumBits = callerSeed ? uint64_t(1) << 63 : uint64_t(0);
    double sum = WaveMultiSum(negativeZero, partition);
    double2 sums = WaveMultiSum(double2(negativeZero, negativeZero), partition);
    bool ok = bit_cast<uint64_t>(sum) == expectedSumBits;
    ok &= bit_cast<uint64_t>(sums.x) == expectedSumBits;
    ok &= bit_cast<uint64_t>(sums.y) == expectedSumBits;
    ok &= bit_cast<uint64_t>(WaveMultiPrefixExclusiveSum(negativeZero, partition)) == uint64_t(0);
    ok &= bit_cast<uint64_t>(WaveMultiPrefixInclusiveSum(negativeZero, partition)) == uint64_t(0);
    double productInput = rank == 0 ? negativeZero : 1.0l;
    ok &= bit_cast<uint64_t>(WaveMultiProduct(productInput, partition)) == (uint64_t(1) << 63);
    double productPrefix = WaveMultiPrefixExclusiveProduct(productInput, partition);
    ok &= bit_cast<uint64_t>(productPrefix) == (rank == 0 ? bit_cast<uint64_t>(1.0l) : uint64_t(1) << 63);
    double infinity = bit_cast<double>(uint64_t(0x7FF00000u) << 32);
    ok &= WaveMultiSum(infinity, partition) == infinity;
    ok &= WaveMultiProduct(infinity, partition) == infinity;
    double quietNaN = bit_cast<double>((uint64_t(0x7FF80000u) << 32) | uint64_t(42));
    ok &= isnan(WaveMultiSum(quietNaN, partition));
    ok &= isnan(WaveMultiProduct(quietNaN, partition));
    return ok;
}

[numthreads(32,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    uint4 singleton = uint4(1u << lane, 0, 0, 0);
    uint64_t bits = payload(lane);
    double value = bit_cast<double>(bits);
    bool ok = bit_cast<uint64_t>(WaveMultiSum(value, singleton)) == bits;
    ok &= bit_cast<uint64_t>(WaveMultiProduct(value, singleton)) == bits;
    double4 vectorValue = double4(bit_cast<double>(payload(lane)), bit_cast<double>(payload(lane + 1)),
        bit_cast<double>(payload(lane + 2)), bit_cast<double>(payload(lane + 3)));
    double4 sum = WaveMultiSum(vectorValue, singleton);
    double4 product = WaveMultiProduct(vectorValue, singleton);
    double2x2 matrixValue = double2x2(vectorValue.x, vectorValue.y, vectorValue.z, vectorValue.w);
    double2x2 matrixSum = WaveMultiSum(matrixValue, singleton);
    double2x2 matrixProduct = WaveMultiProduct(matrixValue, singleton);
    for (uint component = 0; component < 4; ++component)
    {
        uint64_t expected = payload(lane + component);
        ok &= bit_cast<uint64_t>(sum[component]) == expected;
        ok &= bit_cast<uint64_t>(product[component]) == expected;
        ok &= bit_cast<uint64_t>(matrixSum[component / 2][component % 2]) == expected;
        ok &= bit_cast<uint64_t>(matrixProduct[component / 2][component % 2]) == expected;
    }
    ok &= checkZeroPartition(0xFFFFFFFFu, lane, true);
    ok &= checkZeroPartition((lane & 1) == 0 ? 0x55555555u : 0xAAAAAAAAu, lane / 2, false);
    if (lane < 16)
        ok &= checkZeroPartition(0xFFFFu, lane, true);
    if (lane < 13)
        ok &= checkZeroPartition(0x1FFFu, lane, false);
    outputBuffer[lane] = ok ? 5000 + lane : 0;
}

// CHECK: 5000
// CHECK: 5001
// CHECK: 5002
// CHECK: 5003
// CHECK: 5004
// CHECK: 5005
// CHECK: 5006
// CHECK: 5007
// CHECK: 5008
// CHECK: 5009
// CHECK: 5010
// CHECK: 5011
// CHECK: 5012
// CHECK: 5013
// CHECK: 5014
// CHECK: 5015
// CHECK: 5016
// CHECK: 5017
// CHECK: 5018
// CHECK: 5019
// CHECK: 5020
// CHECK: 5021
// CHECK: 5022
// CHECK: 5023
// CHECK: 5024
// CHECK: 5025
// CHECK: 5026
// CHECK: 5027
// CHECK: 5028
// CHECK: 5029
// CHECK: 5030
// CHECK: 5031
