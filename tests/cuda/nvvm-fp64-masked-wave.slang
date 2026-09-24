//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// These closed-form expectations depend only on the caller's partition rank and size.
// Quarter increments above 2^24 and the product's 2^-30 increment are lost in float32.
bool checkScalar(uint4 mask, uint rank, uint count)
{
    double base = 16777216.25l;
    double value = base + double(rank) * 0.25l;
    double factor = 1.0l + 1.0l / 1073741824.0l;
    double productValue = rank == 0 ? factor : 2.0l;
    double prefixSum = double(rank) * base + double(rank * (rank - 1)) * 0.125l;
    double sum = double(count) * base + double(count * (count - 1)) * 0.125l;
    double prefixProduct = rank == 0 ? 1.0l : factor * double(uint64_t(1) << (rank - 1));
    bool ok = WaveMultiSum(value, mask) == sum;
    ok &= WaveMultiProduct(productValue, mask) == factor * double(uint64_t(1) << (count - 1));
    ok &= WaveMultiPrefixExclusiveSum(value, mask) == prefixSum;
    ok &= WaveMultiPrefixInclusiveSum(value, mask) == prefixSum + value;
    ok &= WaveMultiPrefixExclusiveProduct(productValue, mask) == prefixProduct;
    ok &= WaveMultiPrefixInclusiveProduct(productValue, mask) == factor * double(uint64_t(1) << rank);
    return ok;
}

__generic<let N : int>
bool checkVector(uint4 mask, uint rank, uint count)
{
    vector<double, N> value, productValue, expectedSum, expectedPrefixSum;
    vector<double, N> expectedProduct, expectedPrefixProduct, expectedInclusiveProduct;
    for (int component = 0; component < N; ++component)
    {
        double base = (component % 2 == 0 ? 1.0l : -1.0l) * (33554432.25l + double(component) * 0.5l);
        double step = double(component + 1) * 0.125l;
        double factor = 1.0l + double(component + 1) / 1073741824.0l;
        value[component] = base + double(rank) * step;
        productValue[component] = rank == 0 ? factor : 2.0l;
        expectedSum[component] = double(count) * base + double(count * (count - 1)) * 0.5l * step;
        expectedPrefixSum[component] = double(rank) * base + double(rank * (rank - 1)) * 0.5l * step;
        expectedProduct[component] = factor * double(uint64_t(1) << (count - 1));
        expectedPrefixProduct[component] = rank == 0 ? 1.0l : factor * double(uint64_t(1) << (rank - 1));
        expectedInclusiveProduct[component] = factor * double(uint64_t(1) << rank);
    }
    bool ok = all(WaveMultiSum(value, mask) == expectedSum);
    ok &= all(WaveMultiProduct(productValue, mask) == expectedProduct);
    ok &= all(WaveMultiPrefixExclusiveSum(value, mask) == expectedPrefixSum);
    ok &= all(WaveMultiPrefixInclusiveSum(value, mask) == expectedPrefixSum + value);
    ok &= all(WaveMultiPrefixExclusiveProduct(productValue, mask) == expectedPrefixProduct);
    ok &= all(WaveMultiPrefixInclusiveProduct(productValue, mask) == expectedInclusiveProduct);
    return ok;
}

bool checkPartition(uint mask, uint rank, uint count)
{
    uint4 partition = uint4(mask, 0, 0, 0);
    bool ok = checkScalar(partition, rank, count);
    ok &= checkVector<2>(partition, rank, count);
    ok &= checkVector<3>(partition, rank, count);
    ok &= checkVector<4>(partition, rank, count);
    return ok;
}

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    bool ok = checkPartition(0xFFFFFFFFu, lane, 32);
    // Every lane belongs to exactly one sparse partition; all named lanes participate.
    uint residue = lane % 3;
    uint mask = residue == 0 ? 0x49249249u : residue == 1 ? 0x92492492u : 0x24924924u;
    ok &= checkPartition(mask, lane / 3, residue == 2 ? 10 : 11);
    // Only the low thirteen lanes execute; the two interleaved masks remain valid.
    if (lane < 13)
        ok &= checkPartition((lane & 1) == 0 ? 0x1555u : 0x0AAAu, lane / 2, (lane & 1) == 0 ? 7 : 6);
    // A one-lane partition checks both prefix identities in every physical lane.
    ok &= checkPartition(1u << lane, 0, 1);
    outputBuffer[lane] = ok ? 3000 + lane : 0;
}

// CHECK: 3000
// CHECK: 3001
// CHECK: 3002
// CHECK: 3003
// CHECK: 3004
// CHECK: 3005
// CHECK: 3006
// CHECK: 3007
// CHECK: 3008
// CHECK: 3009
// CHECK: 3010
// CHECK: 3011
// CHECK: 3012
// CHECK: 3013
// CHECK: 3014
// CHECK: 3015
// CHECK: 3016
// CHECK: 3017
// CHECK: 3018
// CHECK: 3019
// CHECK: 3020
// CHECK: 3021
// CHECK: 3022
// CHECK: 3023
// CHECK: 3024
// CHECK: 3025
// CHECK: 3026
// CHECK: 3027
// CHECK: 3028
// CHECK: 3029
// CHECK: 3030
// CHECK: 3031
