implementing neural;


namespace neural_unittest
{

// Address type is hardcoded to BindlessAddress<T> because this function is
// [Differentiable] and the autodiff system requires Addr.Differential to also
// conform to IPointerLikeAddress<T>, which can't be expressed as a generic constraint.
[Differentiable]
public void testEncodePerLevel<
        T : __BuiltinFloatingPointType,
        int Dimensions,
        int FeatureDimensionPerEntry,
        uint Log2HashmapSize,
        uint MaxLevels,
        InArray,
        OutArray>
    (no_diff uint level,
     no_diff float maxLevel,
     no_diff float baseScale,
     no_diff float log2PerLevelScale,
     no_diff uint seed,
     InArray position,
     BindlessAddress<T> featureAddress,
     inout OutArray result)
        where T.Differential == T
        where InArray : IArrayAccessor<T>, IDifferentiable
        where OutArray : IArrayAccessor<T>, IDifferentiable
{
    typealias Encoder = PermutoEncoder<T, Dimensions, FeatureDimensionPerEntry, Log2HashmapSize, MaxLevels, InArray, OutArray>;
    Encoder.Params params;
    params.maxLevel = maxLevel;
    Encoder.prepareLevelInfo(level, baseScale, log2PerLevelScale, seed, params.levelInfo);
    Encoder.encodePerLevel<BindlessAddress<T>>(params, position, featureAddress, result);
}

} // namespace neural_unittest
