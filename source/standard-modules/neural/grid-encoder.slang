implementing neural;
#include "common-def.slang"

public enum InterpolationType : uint32_t
{
    Nearest,
    Linear,
    Smoothstep,
    LinearSmoothGrad,
    LinearSmoothGrad2
};

public enum GridType {
    Hash,
    Dense,
    Tiled,
};

// =============================================================================
// Helper Functions
// =============================================================================

/// Computes the scale for a given level.
/// @param level The current level index.
/// @param perLevelScale The per-level scale factor.
/// @param baseResolution The base resolution.
/// @return The scale value (baseResolution * perLevelScale^level - 1).
[ForceInline]
float gridScale(uint level, float log2PerLevelScale, uint baseResolution)
{
    // The -1 means that `baseResolution` refers to the number of grid _vertices_ rather
    // than the number of cells. This results in nice, power-of-2-scaled parameter grids.
    return exp2(float(level) * log2PerLevelScale) * float(baseResolution) - 1.0f;
}

/// Computes the grid resolution from a scale value.
/// @param scale The scale value from gridScale().
/// @return The grid resolution (ceil(scale) + 1).
[ForceInline]
uint gridResolution(float scale)
{
    return uint(ceil(scale)) + 1;
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
    posGrid = uint(tmp);
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
// Grid Encoder
// =============================================================================

VISIBILITY_LEVEL struct GridEncoder<T,
    int Dimensions,
    int FeaturesPerLevel,
    HashType HashType,
    InterpolationType InterpType,
    GridType GridType,
    uint BaseResolution,
    uint TotalNumFeatures,
    uint Log2HashmapSize,
    bool ComputeLevelInfoOnFly = false
    >
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const uint NumLevels = TotalNumFeatures / FeaturesPerLevel;

    /// Precomputed per-level information.
    /// This struct should be populated on the host side for each level.
    /// But if ComputeLevelInfoOnFly is true, we will compute it in the encode function.
    struct LevelInfo
    {
        uint offset;        // Offset into grid storage for this level (in number of features)
        uint hashmapSize;   // Hashmap size for this level
        float scale;        // Scale for computing fractional coordinates
        uint resolution;    // Grid resolution for this level
    }

    struct Params
    {
        uint currentLevel;
        float maxLevel;
        float log2PerLevelScale;   // log2(MaxResolution / BaseResolution) / (NumLevels - 1)
        float interpolationParam;  // Only used for LinearSmoothGrad variants
        LevelInfo levelInfo;       // This field is provided by caller if ComputeLevelInfoOnFly is false
    }

    /// Reads feature values at a given position from the feature table.
    [ForceInline]
    static void readFeatureValue<Storage>(
        Storage featureTable,
        Storage.Address featureTableAddress,
        uint hashmapSize,
        uint resolution,
        uvec<Dimensions> localPos,
        out T result[FeaturesPerLevel])
        where Storage : IStorage<T>
    {
        uint index = gridIndex<HashType, Dimensions>(GridType, hashmapSize, resolution, localPos) * FeaturesPerLevel;

        [ForceUnroll]
        for (uint f = 0; f < FeaturesPerLevel; ++f)
        {
            result[f] = featureTable.read(Storage.getOffset(featureTableAddress, int(index + f)));
        }
    }

    /// Main forward pass: encodes positions using the multi-resolution hash grid.
    ///
    /// @param params Encoding parameters containing:
    ///   - currentLevel: The level to process (replaces blockIdx.y from CUDA version)
    ///   - maxLevel: Maximum level to use (normalized 0-1)
    ///   - interpolationParam: Parameter for LinearSmoothGrad variants
    /// @param position Input position (Dimensions floats)
    /// @param featureTable Storage containing feature vectors for all levels
    /// @param featureTableAddress Base address for feature table storage
    /// @param encodedFeatures Output: encoded feature vector for this position at this level
    static T[FeaturesPerLevel] encode<Storage>(
        no_diff in Params params,
        T position[Dimensions],
        Storage featureTable,
        no_diff in Storage.Address featureTableAddress)
        where Storage : IStorage<T>
    {
        // Initialize output to zero
        T encodedFeatures[FeaturesPerLevel] = {};

        // If level is greater than maxLevel, output zero padding
        if (float(params.currentLevel) >= params.maxLevel + 1e-3f)
        {
            return encodedFeatures;
        }

        // Use precomputed level-specific parameters from constant buffer
        uint levelOffset = params.levelInfo.offset;
        uint hashmapSize = params.levelInfo.hashmapSize;

        // Compute scale and resolution
        float scale = gridScale(params.currentLevel, params.log2PerLevelScale, BaseResolution);
        uint resolution = gridResolution(scale);

        // Offset feature table address to the start of this level
        Storage.Address levelFeatureAddress = Storage.getOffset(featureTableAddress, int(levelOffset * FeaturesPerLevel));

        // Compute fractional coordinates from input position
        vector<float, Dimensions> pos;
        uvec<Dimensions> posGrid;

        [ForceUnroll]
        for (uint dim = 0; dim < Dimensions; ++dim)
        {
            float inputVal = __realCast<float, T>(position[dim]);
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
            readFeatureValue(featureTable, levelFeatureAddress, hashmapSize, resolution, posGrid, encodedFeatures);
            return encodedFeatures;
        }

        // N-linear interpolation
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

            T featureVal[FeaturesPerLevel];
            readFeatureValue(featureTable, levelFeatureAddress, hashmapSize, resolution, posGridLocal, featureVal);

            [ForceUnroll]
            for (uint f = 0; f < FeaturesPerLevel; ++f)
            {
                encodedFeatures[f] += T(weight) * featureVal[f];
            }
        }

        return encodedFeatures;
    }
}
