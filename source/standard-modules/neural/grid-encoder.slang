implementing neural;

public enum InterpolationType : uint32_t
{
    Nearest,
    Linear,
    Smoothstep,
    LinearSmoothGrad,
    LinearSmoothGrad2
};

enum GridType {
    Hash,
    Dense,
    Tiled,
};

// =============================================================================
// Helper Functions
// =============================================================================

/// Type trait for computing grid scale and resolution with compile-time parameters.
/// Usage: GridScaleTraits<BaseResolution, MaxResolution, NumLevels>.scale(level)
///        GridScaleTraits<BaseResolution, MaxResolution, NumLevels>.resolution(level)
internal struct GridScaleTraits<uint BaseResolution, uint MaxResolution, uint NumLevels>
{
    // Compile-time computed log2 per-level scale
    static const float Log2PerLevelScale = log2(float(MaxResolution) / float(BaseResolution)) / float(NumLevels - 1);

    /// Computes the scale for a given level.
    /// @param level The current level index.
    /// @return The scale value (base_resolution * 2^(level * log2_per_level_scale) - 1).
    [ForceInline]
    static float scale(uint level)
    {
        // The -1 means that `baseResolution` refers to the number of grid _vertices_ rather
        // than the number of cells. This results in nice, power-of-2-scaled parameter grids.
        return exp2(float(level) * Log2PerLevelScale) * float(BaseResolution) - 1.0f;
    }

    /// Computes the grid resolution from a scale value.
    /// @param scaleVal The scale value from scale().
    /// @return The grid resolution (ceil(scale) + 1).
    [ForceInline]
    static uint resolution(float scaleVal)
    {
        return uint(ceil(scaleVal)) + 1;
    }

    /// Computes both scale and resolution for a given level.
    /// @param level The current level index.
    /// @param outScale Output: the computed scale value.
    /// @param outResolution Output: the computed resolution.
    [ForceInline]
    static void scaleAndResolution(uint level, out float outScale, out uint outResolution)
    {
        outScale = scale(level);
        outResolution = resolution(outScale);
    }
}

/// Smoothstep interpolation function: 3t^2 - 2t^3
[ForceInline]
float smoothstep(float val)
{
    return val * val * (3.0f - 2.0f * val);
}

/// Derivative of smoothstep: 6t(1 - t)
[ForceInline]
float smoothstepDerivative(float val)
{
    return 6.0f * val * (1.0f - val);
}

/// Second derivative of smoothstep: 6 - 12t
[ForceInline]
float smoothstep2ndDerivative(float val)
{
    return 6.0f - 12.0f * val;
}

/// Identity function (for linear interpolation)
[ForceInline]
float identityFun(float val)
{
    return val;
}

/// Derivative of identity function
[ForceInline]
float identityDerivative(float val)
{
    return 1.0f;
}

/// Second derivative of identity function
[ForceInline]
float identity2ndDerivative(float val)
{
    return 0.0f;
}

/// Linear smooth function for LinearSmoothGrad interpolation
[ForceInline]
float linearSmooth(float val, float lambda)
{
    return identityFun(val) + lambda * smoothstep(val);
}

/// Derivative of linear smooth function
[ForceInline]
float linearSmoothDerivative(float val, float lambda)
{
    return identityDerivative(val) + lambda * smoothstepDerivative(val);
}

/// Computes the grid index for a position, handling hashing when needed.
/// @param gridType The type of grid (Hash, Dense, Tiled).
/// @param hashmapSize The size of the hashmap for this level.
/// @param resolution The grid resolution for this level.
/// @param posGrid The integer grid position.
/// @return The index into the parameter array.
[ForceInline]
uint gridIndex<HashType HashType, int Dimensions>(GridType gridType, uint hashmapSize, uint resolution, uvec<Dimensions> posGrid)
{
    uint stride = 1;
    uint index = 0;

    // The second part of the loop condition avoids integer overflows in finer levels.
    [ForceUnroll]
    for (uint dim = 0; dim < Dimensions && stride <= hashmapSize; ++dim)
    {
        index += posGrid[dim] * stride;
        stride *= resolution;
    }

    if (gridType == GridType.Hash && hashmapSize < stride)
    {
        index = gridHash<HashType, Dimensions>(posGrid);
    }

    return index % hashmapSize;
}

/// Computes the fractional position within a grid cell and the integer grid coordinates.
/// Applies the 0.5 offset for staggering different scales (see Instant NGP paper Appendix A).
[ForceInline]
void posFract<InterpolationType InterpType>(
    float input,
    out float pos,
    out float posDerivative,
    out uint posGrid,
    float scale,
    float interpParam)
{
    // The offset of 0.5 causes different scales to be staggered with respect to each other,
    // preventing spurious alignment of fractional coordinates upon integer scales.
    pos = scale * input + 0.5f;
    float tmp = floor(pos);
    posGrid = uint(int(tmp));
    pos -= tmp;

    // Apply interpolation function and compute derivative
    switch (InterpType)
    {
        case InterpolationType.Nearest:
        case InterpolationType.Linear:
            posDerivative = identityDerivative(pos);
            pos = identityFun(pos);
            break;
        case InterpolationType.Smoothstep:
            posDerivative = smoothstepDerivative(pos);
            pos = smoothstep(pos);
            break;
        case InterpolationType.LinearSmoothGrad:
            // For LinearSmoothGrad, pos remains unchanged (identity) for interpolation,
            // but derivative uses the smooth version
            posDerivative = linearSmoothDerivative(pos, interpParam);
            pos = identityFun(pos);
            break;
        case InterpolationType.LinearSmoothGrad2:
            posDerivative = linearSmoothDerivative(pos, interpParam);
            pos = identityFun(pos);
            break;
    }
}

/// Overload for when we need both pos and pos_for_derivative (LinearSmoothGrad case)
[ForceInline]
void posFractWithDerivativePos<InterpolationType InterpType>(
    float input,
    out float pos,
    out float posForDerivative,
    out float posDerivative,
    out uint posGrid,
    float scale,
    float interpParam)
{
    pos = scale * input + 0.5f;
    float tmp = floor(pos);
    posGrid = uint(int(tmp));
    pos -= tmp;

    switch (InterpType)
    {
        case InterpolationType.Nearest:
        case InterpolationType.Linear:
            posDerivative = identityDerivative(pos);
            pos = identityFun(pos);
            posForDerivative = pos;
            break;
        case InterpolationType.Smoothstep:
            posDerivative = smoothstepDerivative(pos);
            pos = smoothstep(pos);
            posForDerivative = pos;
            break;
        case InterpolationType.LinearSmoothGrad:
            posForDerivative = linearSmooth(pos, interpParam);
            posDerivative = linearSmoothDerivative(pos, interpParam);
            pos = identityFun(pos);
            break;
        case InterpolationType.LinearSmoothGrad2:
            posDerivative = linearSmoothDerivative(pos, interpParam);
            pos = identityFun(pos);
            posForDerivative = pos;
            break;
    }
}

// =============================================================================
// Offset Table Computation
// =============================================================================

/// Computes the number of parameters for a given level.
/// This replicates the host-side offset table computation from tiny-cuda-nn.
[ForceInline]
uint computeParamsInLevel<GridType GridType, uint Log2HashmapSize, uint BaseResolution, uint MaxResolution, uint NumLevels, int Dimensions>(
    uint level)
{
    float scale;
    uint resolution;
    GridScaleTraits<BaseResolution, MaxResolution, NumLevels>.scaleAndResolution(level, scale, resolution);

    // Compute number of dense params: resolution^Dimensions
    // Using a loop to compute power to avoid overflow issues
    uint paramsInLevel = 1;
    [ForceUnroll]
    for (int d = 0; d < Dimensions; ++d)
    {
        // Check for potential overflow
        if (paramsInLevel > 0x7FFFFFFFu / resolution)
        {
            paramsInLevel = 0x7FFFFFFFu;
            break;
        }
        paramsInLevel *= resolution;
    }

    // Align to multiple of 8 for memory access alignment
    paramsInLevel = ((paramsInLevel + 7u) / 8u) * 8u;

    if (GridType == GridType.Dense)
    {
        // Dense grid uses all params
    }
    else if (GridType == GridType.Tiled)
    {
        // Tiled grid limits to base resolution
        uint tiledParams = 1;
        [ForceUnroll]
        for (int d = 0; d < Dimensions; ++d)
        {
            tiledParams *= BaseResolution;
        }
        paramsInLevel = min(paramsInLevel, tiledParams);
    }
    else if (GridType == GridType.Hash)
    {
        // Hash grid limits to 2^Log2HashmapSize
        paramsInLevel = min(paramsInLevel, 1u << Log2HashmapSize);
    }

    return paramsInLevel;
}

/// Type trait for computing Base^Exponent at compile time.
/// Usage: PowerOf<Base, Exponent>.value
/// Note: Exponent must be in range [0, 8] for typical grid dimensions.
internal struct CompileTimePower<uint Base, int Exponent>
{
    // TODO: We don't support recursive type traits yet
    // static const uint Value = (Exponent == 0) ? 1 : PowerOf<Base, Exponent - 1>.Value * Base;
    static const uint Value = (Exponent == 0u) ? 1u :
        Exponent == 1 ? Base :
        Exponent == 2 ? Base * Base :
        Exponent == 3 ? Base * Base * Base :
        Exponent == 4 ? Base * Base * Base * Base :
        Exponent == 5 ? Base * Base * Base * Base * Base :
        Exponent == 6 ? Base * Base * Base * Base * Base * Base :
        Exponent == 7 ? Base * Base * Base * Base * Base * Base * Base :
        Exponent == 8 ? Base * Base * Base * Base * Base * Base * Base * Base :
        0U;
}

/// Computes the offset for a given level by summing params of all previous levels.
///
/// For best performance, consider passing a precomputed offset table instead of
/// using this function. The original CUDA implementation precomputes the offset
/// table on the host side.
///
/// This function has an optimized fast path for Hash grids where all levels are
/// capped at the hashmap size (when BaseResolution^Dimensions >= 2^Log2HashmapSize).
[ForceInline]
uint computeLevelOffset<GridType GridType, uint Log2HashmapSize, uint BaseResolution, uint MaxResolution, uint NumLevels, int Dimensions>(
    uint level)
{
    // Fast path for Hash grids: if base resolution already exceeds hashmap capacity,
    // all levels have the same size, so we can compute offset directly.
    if (GridType == GridType.Hash)
    {
        static const uint hashmapSize = 1u << Log2HashmapSize;
        static const uint basePower = CompileTimePower<BaseResolution, Dimensions>.Value;

        // If base resolution^Dimensions >= hashmap size, all levels are capped
        if (basePower >= hashmapSize)
        {
            return level * hashmapSize;
        }
    }

    // General case: iterate through levels (unrolled at compile time)
    uint offset = 0;
    [ForceUnroll]
    for (uint l = 0; l < NumLevels; ++l)
    {
        if (l >= level)
            break;
        offset += computeParamsInLevel<GridType, Log2HashmapSize, BaseResolution, MaxResolution, NumLevels, Dimensions>(l);
    }
    return offset;
}

/// Computes the hashmap size for a given level.
[ForceInline]
uint computeHashmapSize<GridType GridType, uint Log2HashmapSize, uint BaseResolution, uint MaxResolution, uint NumLevels, int Dimensions>(
    uint level)
{
    return computeParamsInLevel<GridType, Log2HashmapSize, BaseResolution, MaxResolution, NumLevels, Dimensions>(level);
}

// =============================================================================
// Grid Encoder
// =============================================================================

struct GridEncoder<T,
    int Dimensions,
    int FeaturesPerLevel,
    HashType HashType,
    InterpolationType InterpType,
    GridType GridType,
    uint BaseResolution,
    uint MaxResolution,
    uint TotalNumFeatures,
    uint Log2HashmapSize
    >
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const uint NumLevels = TotalNumFeatures / FeaturesPerLevel;

    // Compile-time computed log2 per-level scale
    // Formula: log2(MaxResolution / BaseResolution) / (NumLevels - 1)
    static const float Log2PerLevelScale = log2(float(MaxResolution) / float(BaseResolution)) / float(NumLevels - 1);

    struct Params
    {
        uint numPositions;
        uint currentLevel;
        uint positionIndex;
        float maxLevel;
        float interpolationParam;  // Only used for LinearSmoothGrad variants
    }

    /// Reads grid values at a given position from the grid storage.
    [ForceInline]
    static void readGridValue<Storage>(
        Storage grid,
        Storage.Address gridAddress,
        uint hashmapSize,
        uint resolution,
        uvec<Dimensions> localPos,
        out vector<T, FeaturesPerLevel> result)
        where Storage : IStorage<T>
    {
        uint index = gridIndex<HashType, Dimensions>(GridType, hashmapSize, resolution, localPos) * FeaturesPerLevel;

        [ForceUnroll]
        for (uint f = 0; f < FeaturesPerLevel; ++f)
        {
            result[f] = grid.read(Storage.getOffset(gridAddress, int(index + f)));
        }
    }

    /// Main forward pass: encodes positions using the multi-resolution hash grid.
    ///
    /// @param params Encoding parameters containing:
    ///   - numPositions: Total number of positions to encode
    ///   - currentLevel: The level to process (replaces blockIdx.y from CUDA version)
    ///   - positionIndex: The position index (global thread index / numLevels in the caller)
    ///   - maxLevel: Maximum level to use (normalized 0-1)
    ///   - interpolationParam: Parameter for LinearSmoothGrad variants
    /// @param positions Storage containing input positions (Dimensions floats per position)
    /// @param positionsAddress Base address for positions storage
    /// @param grid Storage containing grid parameters
    /// @param gridAddress Base address for grid storage
    /// @param encodedPositions Output storage for encoded features
    /// @param encodedPositionsAddress Base address for output storage
    static void encode<Storage>(
        no_diff in Params params,
        Storage positions,
        no_diff in Storage.Address positionsAddress,
        Storage grid,
        no_diff in Storage.Address gridAddress,
        Storage encodedPositions,
        no_diff in Storage.Address encodedPositionsAddress)
        where Storage : IStorage<T>
    {
        // Early exit if position index is out of bounds
        if (params.positionIndex >= params.numPositions)
            return;

        // Compute maxLevel in terms of number of levels
        float maxLevelFloat = (params.maxLevel * float(TotalNumFeatures)) / float(FeaturesPerLevel);

        // If level is greater than maxLevel, output zero padding
        if (float(params.currentLevel) >= maxLevelFloat + 1e-3f)
        {
            [ForceUnroll]
            for (uint f = 0; f < FeaturesPerLevel; ++f)
            {
                uint outIdx = params.positionIndex + (params.currentLevel * FeaturesPerLevel + f) * params.numPositions;
                encodedPositions.write(Storage.getOffset(encodedPositionsAddress, int(outIdx)), T(0.0f));
            }
            return;
        }

        // Compute level-specific parameters
        // TODO: These computations are done per-thread. For better performance, consider:
        //   - Precomputing in shared memory at the start of each workgroup
        //   - Using a constant buffer with precomputed offset tables
        uint levelOffset = computeLevelOffset<GridType, Log2HashmapSize, BaseResolution, MaxResolution, NumLevels, Dimensions>(
            params.currentLevel);
        uint hashmapSize = computeHashmapSize<GridType, Log2HashmapSize, BaseResolution, MaxResolution, NumLevels, Dimensions>(
            params.currentLevel);

        // Offset grid address to the start of this level
        Storage.Address levelGridAddress = Storage.getOffset(gridAddress, int(levelOffset * FeaturesPerLevel));

        float scale;
        uint resolution;
        GridScaleTraits<BaseResolution, MaxResolution, NumLevels>.scaleAndResolution(params.currentLevel, scale, resolution);

        // Read input position and compute fractional coordinates
        vector<float, Dimensions> pos;
        uvec<Dimensions> posGrid;

        [ForceUnroll]
        for (uint dim = 0; dim < Dimensions; ++dim)
        {
            T readVal = positions.read(Storage.getOffset(positionsAddress, int(dim * params.numPositions + params.positionIndex)));
            float inputVal = __realCast<float, T>(readVal);
            float posDerivative;  // Not used, but required by posFract
            posFract<InterpType>(
                inputVal,
                pos[dim],
                posDerivative,
                posGrid[dim],
                scale,
                params.interpolationParam);
        }

        // Handle nearest neighbor interpolation (no interpolation)
        if (InterpType == InterpolationType.Nearest)
        {
            vector<T, FeaturesPerLevel> result;
            readGridValue(grid, levelGridAddress, hashmapSize, resolution, posGrid, result);

            [ForceUnroll]
            for (uint f = 0; f < FeaturesPerLevel; ++f)
            {
                uint outIdx = params.positionIndex + (params.currentLevel * FeaturesPerLevel + f) * params.numPositions;
                encodedPositions.write(Storage.getOffset(encodedPositionsAddress, int(outIdx)), result[f]);
            }
            return;
        }

        // N-linear interpolation
        vector<T, FeaturesPerLevel> result = vector<T, FeaturesPerLevel>(T(0.0f));

        // Iterate over all 2^Dimensions corners of the grid cell
        [ForceUnroll]
        for (uint idx = 0; idx < (1u << Dimensions); ++idx)
        {
            float weight = 1.0f;
            uvec<Dimensions> posGridLocal;

            [ForceUnroll]
            for (uint dim = 0; dim < Dimensions; ++dim)
            {
                if ((idx & (1u << dim)) == 0)
                {
                    weight *= 1.0f - pos[dim];
                    posGridLocal[dim] = posGrid[dim];
                }
                else
                {
                    weight *= pos[dim];
                    posGridLocal[dim] = posGrid[dim] + 1;
                }
            }

            vector<T, FeaturesPerLevel> gridVal;
            readGridValue(grid, levelGridAddress, hashmapSize, resolution, posGridLocal, gridVal);

            [ForceUnroll]
            for (uint f = 0; f < FeaturesPerLevel; ++f)
            {
                result[f] += T(weight) * gridVal[f];
            }
        }

        // Write encoded positions
        [ForceUnroll]
        for (uint f = 0; f < FeaturesPerLevel; ++f)
        {
            uint outIdx = params.positionIndex + (params.currentLevel * FeaturesPerLevel + f) * params.numPositions;
            encodedPositions.write(Storage.getOffset(encodedPositionsAddress, int(outIdx)), result[f]);
        }
    }
}
