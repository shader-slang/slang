//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute-dispatch 4,1,1 -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute-dispatch 4,1,1 -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute-dispatch 4,1,1 -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Wide bases and low-word carries distinguish exact64-bit arithmetic from truncated shuffles.
// Signed cases stay in range; only the separate unsigned case below intentionally wraps.
__generic<T : __BuiltinArithmeticType>
T baseValue(uint component)
{
    T base = bit_cast<T>((uint64_t(1) << 35) + uint64_t(0xFFFFFFF0u) + uint64_t(component));
    return T is int64_t ? -base : base;
}

__generic<T : __BuiltinArithmeticType>
T productFactor(uint component)
{
    T factor = bit_cast<T>((uint64_t(1) << 33) + 3 + uint64_t(component));
    return T is int64_t ? -factor : factor;
}

__generic<T : __BuiltinArithmeticType>
bool checkScalar(uint4 mask, uint rank, uint count)
{
    T base = baseValue<T>(0);
    T value = base + T(rank);
    T prefix = T(rank) * base + T(rank * (rank - 1) / 2);
    T sum = T(count) * base + T(count * (count - 1) / 2);
    T factor = productFactor<T>(0);
    T productValue = rank == 0 ? factor : rank == 1 ? T(3) : T(1);
    T product = count == 1 ? factor : factor * T(3);
    T prefixProduct = rank == 0 ? T(1) : rank == 1 ? factor : factor * T(3);
    bool ok = WaveMultiSum(value, mask) == sum;
    ok &= WaveMultiProduct(productValue, mask) == product;
    ok &= WaveMultiPrefixExclusiveSum(value, mask) == prefix;
    ok &= WaveMultiPrefixInclusiveSum(value, mask) == prefix + value;
    ok &= WaveMultiPrefixExclusiveProduct(productValue, mask) == prefixProduct;
    ok &= WaveMultiPrefixInclusiveProduct(productValue, mask) == prefixProduct * productValue;
    return ok;
}

__generic<T : __BuiltinArithmeticType, let N : int>
bool checkVector(uint4 mask, uint rank, uint count)
{
    vector<T, N> value, productValue, sum, prefix, product, prefixProduct;
    for (int component = 0; component < N; ++component)
    {
        T base = baseValue<T>(uint(component));
        T step = T(component + 1);
        value[component] = base + T(rank) * step;
        prefix[component] = T(rank) * base + T(rank * (rank - 1) / 2) * step;
        sum[component] = T(count) * base + T(count * (count - 1) / 2) * step;
        T factor = productFactor<T>(uint(component));
        productValue[component] = rank == 0 ? factor : rank == 1 ? T(3) : T(1);
        product[component] = count == 1 ? factor : factor * T(3);
        prefixProduct[component] = rank == 0 ? T(1) : rank == 1 ? factor : factor * T(3);
    }
    bool ok = all(WaveMultiSum(value, mask) == sum);
    ok &= all(WaveMultiProduct(productValue, mask) == product);
    ok &= all(WaveMultiPrefixExclusiveSum(value, mask) == prefix);
    ok &= all(WaveMultiPrefixInclusiveSum(value, mask) == prefix + value);
    ok &= all(WaveMultiPrefixExclusiveProduct(productValue, mask) == prefixProduct);
    ok &= all(WaveMultiPrefixInclusiveProduct(productValue, mask) == prefixProduct * productValue);
    return ok;
}

__generic<T : __BuiltinArithmeticType>
bool sameMatrix(matrix<T, 2, 2> actual, matrix<T, 2, 2> expected)
{
    return all(actual[0] == expected[0]) & all(actual[1] == expected[1]);
}

__generic<T : __BuiltinArithmeticType>
bool checkMatrix(uint4 mask, uint rank, uint count)
{
    T base = baseValue<T>(0);
    T x = base + T(rank);
    T s = T(count) * base + T(count * (count - 1) / 2);
    T p = T(rank) * base + T(rank * (rank - 1) / 2);
    matrix<T, 2, 2> value = matrix<T, 2, 2>(x, x * T(2), x * T(3), x * T(4));
    matrix<T, 2, 2> sum = matrix<T, 2, 2>(s, s * T(2), s * T(3), s * T(4));
    matrix<T, 2, 2> prefix = matrix<T, 2, 2>(p, p * T(2), p * T(3), p * T(4));
    T factor = productFactor<T>(0);
    T v = rank == 0 ? factor : T(1);
    matrix<T, 2, 2> factors = matrix<T, 2, 2>(v, T(1), T(1), v);
    matrix<T, 2, 2> product = matrix<T, 2, 2>(factor, T(1), T(1), factor);
    T f = rank == 0 ? T(1) : factor;
    matrix<T, 2, 2> prefixProduct = matrix<T, 2, 2>(f, T(1), T(1), f);
    bool ok = sameMatrix(WaveMultiSum(value, mask), sum);
    ok &= sameMatrix(WaveMultiProduct(factors, mask), product);
    ok &= sameMatrix(WaveMultiPrefixExclusiveSum(value, mask), prefix);
    ok &= sameMatrix(WaveMultiPrefixInclusiveSum(value, mask), prefix + value);
    ok &= sameMatrix(WaveMultiPrefixExclusiveProduct(factors, mask), prefixProduct);
    ok &= sameMatrix(WaveMultiPrefixInclusiveProduct(factors, mask), product);
    return ok;
}

__generic<T : __BuiltinArithmeticType>
bool checkPartition(uint mask, uint rank, uint count)
{
    uint4 partition = uint4(mask, 0, 0, 0);
    bool ok = checkScalar<T>(partition, rank, count);
    ok &= checkVector<T, 2>(partition, rank, count);
    ok &= checkVector<T, 3>(partition, rank, count);
    ok &= checkVector<T, 4>(partition, rank, count);
    ok &= checkMatrix<T>(partition, rank, count);
    return ok;
}

__generic<T : __BuiltinArithmeticType>
bool checkActive(uint rank, uint count)
{
    T base = baseValue<T>(0);
    T value = base + T(rank);
    T sum = T(count) * base + T(count * (count - 1) / 2);
    T prefix = T(rank) * base + T(rank * (rank - 1) / 2);
    T factor = productFactor<T>(0);
    T p = rank == 0 ? factor : T(1);
    bool ok = WaveActiveSum(value) == sum;
    ok &= WavePrefixSum(value) == prefix;
    ok &= WaveActiveProduct(p) == factor;
    ok &= WavePrefixProduct(p) == (rank == 0 ? T(1) : factor);
    vector<T, 3> wide = vector<T, 3>(value, value * T(2), value * T(3));
    ok &= all(WaveActiveSum(wide) == vector<T, 3>(sum, sum * T(2), sum * T(3)));
    return ok;
}

// Four groups cover full, sparse, partial and singleton partitions through the same body.
// Runtime group selection avoids duplicating every inlined fold for each mask scenario.
[numthreads(32, 1, 1)]
void computeMain(uint3 group : SV_GroupID, uint lane : SV_GroupIndex)
{
    uint scenario = group.x;
    uint residue = lane % 3;
    uint sparseMask = residue == 0 ? 0x49249249u : residue == 1 ? 0x92492492u : 0x24924924u;
    uint partialMask = (lane & 1) == 0 ? 0x1555u : 0x0AAAu;
    uint mask = scenario == 0 ? 0xFFFFFFFFu : scenario == 1 ? sparseMask :
        scenario == 2 ? partialMask : 1u << lane;
    uint rank = scenario == 0 ? lane : scenario == 1 ? lane / 3 :
        scenario == 2 ? lane / 2 : 0;
    uint count = scenario == 0 ? 32 : scenario == 1 ? (residue == 2 ? 10 : 11) :
        scenario == 2 ? ((lane & 1) == 0 ? 7 : 6) : 1;
    bool ok = true;
    if (scenario != 2 || lane < 13)
    {
        ok = checkPartition<int64_t>(mask, rank, count);
        ok &= checkPartition<uint64_t>(mask, rank, count);
        uint activeCount = scenario == 2 ? 13 : 32;
        ok &= checkActive<int64_t>(lane, activeCount);
        ok &= checkActive<uint64_t>(lane, activeCount);
    }
    uint64_t wrapBase = (uint64_t(1) << 63) + uint64_t(0xFFFFFFF0u);
    ok &= WaveActiveSum(wrapBase + uint64_t(lane)) == wrapBase * uint64_t(32) + 496;
    bool allPassed = WaveActiveAllTrue(ok);
    if (lane == 0)
        outputBuffer[scenario] = allPassed ? 5000 + scenario : 0;
}

// CHECK: 5000
// CHECK: 5001
// CHECK: 5002
// CHECK: 5003
