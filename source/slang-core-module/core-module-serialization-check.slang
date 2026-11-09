// This build-time check verifies that interface static constants survive core-module serialization.
// A malformed core archive can otherwise defer the failure until user code specializes a generic
// function that folds a constraint-provided constant.
interface ICoreModuleSerializationKey
{
    static const uint bitCount;
}

struct CoreModuleSerializationKey : ICoreModuleSerializationKey
{
    static const uint bitCount = 64;
}

struct CoreModuleSerializationSorter<TKey : ICoreModuleSerializationKey>
{
    void sort()
    {
        static_assert(TKey.bitCount <= 64, "core-module static constant did not round-trip");
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void coreModuleSerializationCheck()
{
    CoreModuleSerializationSorter<CoreModuleSerializationKey> sorter;
    sorter.sort();
}
