implementing neural;


namespace neural_unittest
{

public uint32_t testPrimeHash<int Dimensions>(Array<uint32_t, Dimensions> posGrid)
{
    return primeHash<Dimensions>(posGrid);
}

public uint32_t testCoherentPrimeHash<int Dimensions>(Array<uint32_t, Dimensions> posGrid)
{
    return coherentPrimeHash<Dimensions>(posGrid);
}

public uint32_t testReversedPrimeHash<int Dimensions>(Array<uint32_t, Dimensions> posGrid)
{
    return reversedPrimeHash<Dimensions>(posGrid);
}

public uint32_t testBaseConvertHash<int Dimensions>(Array<uint32_t, Dimensions> posGrid)
{
    return baseConvertHash<Dimensions>(posGrid);
}

public uint32_t testRngHash<int Dimensions>(Array<uint32_t, Dimensions> posGrid)
{
    return rngHash<Dimensions>(posGrid);
}

public uint32_t testGridHashPrime<int Dimensions>(Array<uint32_t, Dimensions> posGrid)
{
    return gridHash<HashType.Prime, Dimensions>(posGrid);
}

public uint32_t testGridHashCoherentPrime<int Dimensions>(Array<uint32_t, Dimensions> posGrid)
{
    return gridHash<HashType.CoherentPrime, Dimensions>(posGrid);
}

public uint32_t testGridHashReversedPrime<int Dimensions>(Array<uint32_t, Dimensions> posGrid)
{
    return gridHash<HashType.ReversedPrime, Dimensions>(posGrid);
}

public uint32_t testGridHashBaseConvert<int Dimensions>(Array<uint32_t, Dimensions> posGrid)
{
    return gridHash<HashType.BaseConvert, Dimensions>(posGrid);
}

public uint32_t testGridHashRng<int Dimensions>(Array<uint32_t, Dimensions> posGrid)
{
    return gridHash<HashType.Rng, Dimensions>(posGrid);
}

} // namespace neural_unittest
