implementing neural;
#include "common-def.slang"

// =============================================================================
// Permuto Encoder
// =============================================================================

public struct PermutoEncoder<T,
    int Dimensions,
    int FeaturesPerLevel,
    uint TotalNumFeatures,
    uint Log2HashmapSize
    >
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    static const uint NumLevels = TotalNumFeatures / FeaturesPerLevel;

    /// Precomputed per-level information.
    VISIBILITY_LEVEL struct LevelInfo
    {
        VISIBILITY_LEVEL uint offset;        // Offset into feature table for this level (in number of features)
        VISIBILITY_LEVEL uint hashmapSize;   // Hashmap size for this level
    }

    VISIBILITY_LEVEL struct Params
    {
        VISIBILITY_LEVEL uint currentLevel;
        VISIBILITY_LEVEL float maxLevel;
        VISIBILITY_LEVEL float baseScale;           // Base scale for the encoding
        VISIBILITY_LEVEL float log2PerLevelScale;   // log2(MaxResolution / BaseResolution) / (NumLevels - 1)
        VISIBILITY_LEVEL LevelInfo levelInfo;
    }

    // =========================================================================
    // Permutohedral Lattice Helper Functions
    // =========================================================================

    /// Computes the permutohedral lattice index using base conversion hash.
    /// @param key The lattice coordinate.
    /// @param hashmapSize The size of the hashmap.
    /// @return The index into the parameter array.
    [ForceInline]
    static uint permutoIndex(uvec<Dimensions> key, uint hashmapSize)
    {
        return baseConvertHash<Dimensions>(key) % hashmapSize;
    }

    /// Elevates a D-dimension vector to (D+1)-dimension homogeneous vector on hyperplane H_d.
    /// The sum of the components of `elevated` is zero, ensuring it's within hyperplane H_d.
    /// The magnitudes of the components of `elevated` are similar to each other.
    /// @param pos Input position [Dimensions].
    /// @param scalesPerDim Per-dimension scaling factors [Dimensions].
    /// @param shiftsPerDim Per-dimension shifts [Dimensions].
    /// @param elevated Output elevated coordinates [Dimensions+1].
    [ForceInline]
    static void permutoElevate(
        float pos[Dimensions],
        float scalesPerDim[Dimensions],
        float shiftsPerDim[Dimensions],
        out float elevated[Dimensions + 1])
    {
        float sum = 0.0f;
        [ForceUnroll]
        for (int dim = Dimensions - 1; dim >= 0; --dim)
        {
            float cf = (pos[dim] + shiftsPerDim[dim]) * scalesPerDim[dim];
            elevated[dim + 1] = sum - float(dim + 1) * cf;
            sum += cf;
        }
        elevated[0] = sum;
    }

    /// Finds the closest remainder-0 point and computes the rank ordering.
    /// @param elevated The elevated coordinates [Dimensions+1].
    /// @param rem0 Output: coordinates of remainder-0 point [Dimensions+1].
    /// @param rank Output: rank ordering [Dimensions+1].
    [ForceInline]
    static void permutoFindRem0(
        float elevated[Dimensions + 1],
        out int rem0[Dimensions + 1],
        out int rank[Dimensions + 1])
    {
        // Find the closest remainder-0 point through rounding
        int sum = 0;
        [ForceUnroll]
        for (uint dim = 0; dim <= Dimensions; ++dim)
        {
            // Using xxx*(1.0f/N) is faster than xxx/N
            float v = elevated[dim] * (1.0f / float(Dimensions + 1));
            float up = ceil(v) * float(Dimensions + 1);
            float down = floor(v) * float(Dimensions + 1);
            if (up - elevated[dim] < elevated[dim] - down)
            {
                rem0[dim] = int(up);
            }
            else
            {
                rem0[dim] = int(down);
            }
            sum += rem0[dim];
        }
        sum /= int(Dimensions + 1);

        // Find the simplex we are in and store it in rank
        // (where rank describes what position coordinate i has in the sorted order)
        [ForceUnroll]
        for (uint dim = 0; dim < Dimensions; ++dim)
        {
            float di = elevated[dim] - float(rem0[dim]);
            [MaxIters(Dimensions)]
            for (uint otherDim = dim + 1; otherDim <= Dimensions; ++otherDim)
            {
                if (di < elevated[otherDim] - float(rem0[otherDim]))
                {
                    rank[dim]++;
                }
                else
                {
                    rank[otherDim]++;
                }
            }
        }

        // If the point doesn't lie on the plane (sum != 0) bring it back
        [ForceUnroll]
        for (uint dim = 0; dim <= Dimensions; ++dim)
        {
            rank[dim] += sum;
            if (rank[dim] < 0)
            {
                rank[dim] += int(Dimensions + 1);
                rem0[dim] += int(Dimensions + 1);
            }
            else if (rank[dim] > int(Dimensions))
            {
                rank[dim] -= int(Dimensions + 1);
                rem0[dim] -= int(Dimensions + 1);
            }
        }
    }

    /// Computes the barycentric coordinates for permutohedral interpolation.
    /// See p.10 in [Adams et al. 2010].
    /// @param elevated The elevated coordinates [Dimensions+1].
    /// @param rem0 The remainder-0 point coordinates [Dimensions+1].
    /// @param rank The rank ordering [Dimensions+1].
    /// @param barycentric Output: barycentric coordinates [Dimensions+2].
    [ForceInline]
    static void permutoBarycentric(
        float elevated[Dimensions + 1],
        int rem0[Dimensions + 1],
        int rank[Dimensions + 1],
        out float barycentric[Dimensions + 2])
    {
        // Initialize barycentric to zero
        [ForceUnroll]
        for (uint i = 0; i < Dimensions + 2; ++i)
        {
            barycentric[i] = 0.0f;
        }

        // Compute the barycentric coordinates
        [ForceUnroll]
        for (uint dim = 0; dim <= Dimensions; ++dim)
        {
            float delta = (elevated[dim] - float(rem0[dim])) * (1.0f / float(Dimensions + 1));
            barycentric[int(Dimensions) - rank[dim]] += delta;
            barycentric[int(Dimensions + 1) - rank[dim]] -= delta;
        }
        // Wrap around
        barycentric[0] += 1.0f + barycentric[Dimensions + 1];
    }

    // =========================================================================
    // Feature Reading and Encoding
    // =========================================================================

    /// Reads feature values at a given lattice position from the feature table.
    [ForceInline]
    static void readFeatureValue<Storage>(
        Storage featureTable,
        Storage.Address featureTableAddress,
        uint hashmapSize,
        uvec<Dimensions> key,
        out T result[FeaturesPerLevel])
        where Storage : IStorage<T>
    {
        uint index = permutoIndex(key, hashmapSize) * FeaturesPerLevel;

        [ForceUnroll]
        for (uint f = 0; f < FeaturesPerLevel; ++f)
        {
            result[f] = featureTable.read(Storage.getOffset(featureTableAddress, int(index + f)));
        }
    }

    /// Main forward pass: encodes positions using the permutohedral lattice.
    ///
    /// @param params Encoding parameters containing:
    ///   - currentLevel: The level to process
    ///   - maxLevel: Maximum level to use
    ///   - baseScale: Base scale for the encoding
    ///   - log2PerLevelScale: Log2 of per-level scale factor
    /// @param position Input position (Dimensions floats)
    /// @param scalesPerDim Per-dimension scaling factors (precomputed based on level)
    /// @param shiftsPerDim Per-dimension random shifts (precomputed based on level)
    /// @param featureTable Storage containing feature vectors for all levels
    /// @param featureTableAddress Base address for feature table storage
    /// @return Encoded feature vector for this position at this level
    VISIBILITY_LEVEL static T[FeaturesPerLevel] encode<Storage>(
        no_diff in Params params,
        T position[Dimensions],
        float scalesPerDim[Dimensions],
        float shiftsPerDim[Dimensions],
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

        // Use precomputed level-specific parameters
        uint levelOffset = params.levelInfo.offset;
        uint hashmapSize = params.levelInfo.hashmapSize;

        // Offset feature table address to the start of this level
        Storage.Address levelFeatureAddress = Storage.getOffset(featureTableAddress, int(levelOffset * FeaturesPerLevel));

        // Convert position to float array
        float pos[Dimensions];
        [ForceUnroll]
        for (uint dim = 0; dim < Dimensions; ++dim)
        {
            pos[dim] = __realCast<float, T>(position[dim]);
        }

        // Elevate D-dimension vector to (D+1)-dimension homogeneous vector on hyperplane H_d
        float elevated[Dimensions + 1];
        permutoElevate(pos, scalesPerDim, shiftsPerDim, elevated);

        // Find the closest remainder-0 and rank
        int rem0[Dimensions + 1];
        int rank[Dimensions + 1] = {};
        permutoFindRem0(elevated, rem0, rank);

        // Compute the barycentric coordinates
        float barycentric[Dimensions + 2];
        permutoBarycentric(elevated, rem0, rank, barycentric);

        // Interpolate the values using barycentric weights
        uvec<Dimensions> key;
        [ForceUnroll]
        for (uint k = 0; k <= Dimensions; ++k)  // For each remainder-k vertex
        {
            // Compute the coordinates of the remainder-k vertex
            [ForceUnroll]
            for (uint dim = 0; dim < Dimensions; ++dim)
            {
                key[dim] = uint(rem0[dim] + int(k));
                if (rank[dim] > int(Dimensions - k))
                {
                    key[dim] -= uint(Dimensions + 1);
                }
            }

            // Read feature value at this vertex
            T featureVal[FeaturesPerLevel];
            readFeatureValue(featureTable, levelFeatureAddress, hashmapSize, key, featureVal);

            // Accumulate with barycentric weight
            float weight = barycentric[k];
            [ForceUnroll]
            for (uint f = 0; f < FeaturesPerLevel; ++f)
            {
                encodedFeatures[f] += T(weight) * featureVal[f];
            }
        }

        return encodedFeatures;
    }

    /// Convenience overload that computes scalesPerDim and shiftsPerDim internally.
    /// Note: This requires a seed for the random shift generation.
    VISIBILITY_LEVEL static T[FeaturesPerLevel] encode<Storage>(
        no_diff in Params params,
        T position[Dimensions],
        uint randomSeed,
        Storage featureTable,
        no_diff in Storage.Address featureTableAddress)
        where Storage : IStorage<T>
    {
        // Compute scale for this level
        float scale = params.baseScale * exp2(float(params.currentLevel) * params.log2PerLevelScale);

        // Compute per-dimension scales and shifts
        float scalesPerDim[Dimensions];
        float shiftsPerDim[Dimensions];

        // Initialize RNG for random shifts (different levels should draw differently)
        Pcg32 rng = Pcg32(randomSeed);
        rng.advance(int64_t(params.currentLevel * Dimensions));

        [ForceUnroll]
        for (uint dim = 0; dim < Dimensions; ++dim)
        {
            scalesPerDim[dim] = scale * rsqrt(float((dim + 1) * (dim + 2)));
            // Convert uint32 to float in [0, 1) then scale to [-5, 5)
            float randFloat = float(rng.nextUint()) * (1.0f / 4294967296.0f);
            shiftsPerDim[dim] = randFloat * 10.0f - 5.0f;
        }

        return encode(params, position, scalesPerDim, shiftsPerDim, featureTable, featureTableAddress);
    }
}
