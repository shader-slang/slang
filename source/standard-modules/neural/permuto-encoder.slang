implementing neural;
#include "common-def.slang"

// =============================================================================
// Permuto Encoder
// =============================================================================

public struct PermutoEncoder<T,
    int Dimensions,
    int FeatureDimensionPerEntry,
    uint Log2HashmapSize,
    uint MaxLevels,
    InArray,
    OutArray
    > :ITrainableEncoder<T, InArray, OutArray>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
    where InArray : IArrayAccessor<T>, IDifferentiable
    where OutArray : IArrayAccessor<T>, IDifferentiable
{
    /// Precomputed per-level information.
    public struct PerLevelInfo
    {
        public uint currentLevelIndex;
        public uint offsetOfFeatureTable;         // Offset into feature table for this level (in number of features)
        public uint featureTableSize;             // Hashmap size for this level
        public float scale;                       // Scale for this level
        public float scalesPerDim[Dimensions];
        public float shiftsPerDim[Dimensions];
    }

    VISIBILITY_LEVEL struct Params
    {
        VISIBILITY_LEVEL float maxLevel;                // It's maxLevelRatio * MaxLevels, where maxLevelRatio is a user defined ratio
        VISIBILITY_LEVEL PerLevelInfo levelInfo;        // Contains scale, offset, hashmap size, and per-dim scales/shifts
    }

    public struct HyperParameters
    {
        public float maxLevel;
        public PerLevelInfo[MaxLevels] levelInfo;
    };

    // =========================================================================
    // Permutohedral Lattice Helper Functions
    // =========================================================================

    /// Computes the permutohedral lattice index using base conversion hash.
    /// @param key The lattice coordinate.
    /// @param hashmapSize The size of the hashmap.
    /// @return The index into the parameter array.
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
    [Differentiable]
    static void permutoElevate<InArray>(
        InArray pos,
        no_diff in float scalesPerDim[Dimensions],
        no_diff in float shiftsPerDim[Dimensions],
        out float elevated[Dimensions + 1])
            where InArray : IArrayAccessor<T>
            where InArray : IDifferentiable
    {
        float sum = 0.0f;
        [ForceUnroll]
        for (int dim = Dimensions - 1; dim >= 0; --dim)
        {
            float cf = (__realCast<float>(pos[dim]) + shiftsPerDim[dim]) * scalesPerDim[dim];
            elevated[dim + 1] = sum - float(dim + 1) * cf;
            sum += cf;
        }
        elevated[0] = sum;
    }

    /// Finds the closest remainder-0 point and computes the rank ordering.
    /// @param elevated The elevated coordinates [Dimensions+1].
    /// @param rem0 Output: coordinates of remainder-0 point [Dimensions+1].
    /// @param rank Output: rank ordering [Dimensions+1].
    /// Note: Not differentiable since outputs are int (no gradient needed).
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
    [Differentiable]
    static void permutoBarycentric(
        float elevated[Dimensions + 1],
        int rem0[Dimensions + 1],
        int rank[Dimensions + 1],
        out float barycentric[Dimensions + 2])
    {
        // Compute the barycentric coordinates
        [ForceUnroll]
        for (uint dim = 0; dim <= Dimensions; ++dim)
        {
            float delta = (elevated[dim] - float(rem0[dim])) * (1.0f / float(Dimensions + 1));
            int idxPlus = int(Dimensions) - rank[dim];
            int idxMinus = int(Dimensions + 1) - rank[dim];
            barycentric[idxPlus] += delta;
            barycentric[idxMinus] -= delta;
        }
        // Wrap around
        barycentric[0] += 1.0f + barycentric[Dimensions + 1];
    }

    // =========================================================================
    // Feature Reading and Encoding
    // =========================================================================

    /// Reads feature values at a given lattice position from the feature table.
    [BackwardDerivative(readFeatureValueBwd)]
    static void readFeatureValue<Storage>(
        Storage featureTable,
        no_diff in Storage.Address featureTableAddress,
        uint hashmapSize,
        uvec<Dimensions> key,
        out T result[FeatureDimensionPerEntry])
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
    {
        uint index = permutoIndex(key, hashmapSize) * FeatureDimensionPerEntry;

        [ForceUnroll]
        for (uint f = 0; f < FeatureDimensionPerEntry; ++f)
        {
            result[f] = featureTable.read(Storage.getOffset(featureTableAddress, int(index + f)));
        }
    }

    static void readFeatureValueBwd<Storage>(
        DifferentialPtrPair<Storage> featureTable,
        no_diff in Storage.Address featureTableAddress,
        uint hashmapSize,
        uvec<Dimensions> key,
        T.Differential[FeatureDimensionPerEntry] result)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
    {
        uint index = permutoIndex(key, hashmapSize) * FeatureDimensionPerEntry;

        [ForceUnroll]
        for (uint f = 0; f < FeatureDimensionPerEntry; ++f)
        {
            let offset = Storage.getOffset(featureTableAddress, int(index + f));
            featureTable.d.atomicAdd(offset, result[f]);
        }
    }


    /// Main forward pass: encodes positions using the permutohedral lattice for a single level.
    /// Writes results to the appropriate offset in the full output array.
    ///
    /// @param params Encoding parameters containing:
    ///   - maxLevel: Maximum level to use (maxLevelRatio * MaxLevels)
    ///   - levelInfo: Per-level info (currentLevelIndex, offsetOfFeatureTable, featureTableSize, scale, scalesPerDim, shiftsPerDim)
    /// @param position Input position (Dimensions floats)
    /// @param featureTable Storage containing feature vectors for all levels
    /// @param featureTableAddress Base address for feature table storage
    /// @param encodedFeatures Output array of size MaxLevels * FeatureDimensionPerEntry. Results are written at offset currentLevelIndex * FeatureDimensionPerEntry.
    [Differentiable]
    VISIBILITY_LEVEL static void encodePerLevel<Storage>(
        no_diff in Params params,
        InArray position,
        Storage featureTable,
        no_diff in Storage.Address featureTableAddress,
        inout OutArray encodedFeatures)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
    {
        // Compute offset into output array for this level
        uint outputOffset = params.levelInfo.currentLevelIndex * FeatureDimensionPerEntry;

        // Initialize this level's output to zero
        [ForceUnroll]
        for (uint f = 0; f < FeatureDimensionPerEntry; ++f)
        {
            encodedFeatures[outputOffset + f] = T(0);
        }

        // If level is greater than maxLevel, output zero padding (already initialized above)
        if (float(params.levelInfo.currentLevelIndex) >= params.maxLevel + 1e-3f)
        {
            return;
        }

        // Use precomputed level-specific parameters
        uint levelOffset = params.levelInfo.offsetOfFeatureTable;
        uint hashmapSize = params.levelInfo.featureTableSize;

        // Offset feature table address to the start of this level
        Storage.Address levelFeatureAddress = Storage.getOffset(featureTableAddress, int(levelOffset * FeatureDimensionPerEntry));

        // Elevate D-dimension vector to (D+1)-dimension homogeneous vector on hyperplane H_d
        float elevated[Dimensions + 1];
        permutoElevate(position, params.levelInfo.scalesPerDim, params.levelInfo.shiftsPerDim, elevated);

        // Find the closest remainder-0 and rank
        int rem0[Dimensions + 1];
        int rank[Dimensions + 1] = {};
        permutoFindRem0(elevated, rem0, rank);

        // Compute the barycentric coordinates
        float barycentric[Dimensions + 2] = {};
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
            T featureVal[FeatureDimensionPerEntry];
            readFeatureValue(featureTable, levelFeatureAddress, hashmapSize, key, featureVal);

            // Accumulate with barycentric weight
            float weight = barycentric[k];
            [ForceUnroll]
            for (uint f = 0; f < FeatureDimensionPerEntry; ++f)
            {
                encodedFeatures[outputOffset + f] += T(weight) * featureVal[f];
            }
        }
    }

    [Differentiable]
    public static OutArray encode<Storage>(HyperParameters params, in InArray input, Storage parameters, no_diff in Storage.Address parametersAddress)
        where Storage : IStorage<T>
        where Storage.Differential : IStorage<T.Differential>
        where Storage.Address == Storage.Differential.Address
    {
        OutArray encodedFeatures = OutArray();

        Params levelParams;
        levelParams.maxLevel = params.maxLevel;

        [ForceUnroll]
        for (uint level = 0; level < MaxLevels; ++level)
        {
            levelParams.levelInfo = params.levelInfo[level];
            encodePerLevel<Storage>(levelParams, input, parameters, parametersAddress, encodedFeatures);
        }

        return encodedFeatures;
    }

    [Differentiable]
    public static OutArray encode<Address>(HyperParameters params, in InArray input, Address parametersAddress)
        where Address : IPointerLikeAddress<T>
    {
        OutArray encodedFeatures = OutArray();
        static_assert(false, "Not implemented");
        return encodedFeatures;
    }

    /// Helper function to compute the PerLevelInfo (offset, hashmapSize, etc.) for a given level.
    /// The hashmap size is 2^Log2HashmapSize for all levels.
    /// The offset for each level is currentLevel * hashmapSize.
    ///
    /// @param currentLevel The current level being processed
    /// @param baseScale Base scale for the encoding
    /// @param log2PerLevelScale Log2 of the per-level scale factor
    /// @param seed Seed for the random shift generation
    /// @param levelInfo Output: the computed level info with all parameters
    VISIBILITY_LEVEL static void prepareLevelInfo(
        in uint currentLevel,
        in float baseScale,
        in float log2PerLevelScale,
        in uint seed,
        out PerLevelInfo levelInfo)
    {
        static const uint HashmapSize = 1u << Log2HashmapSize;
        levelInfo.currentLevelIndex = currentLevel;
        levelInfo.featureTableSize = HashmapSize;
        levelInfo.offsetOfFeatureTable = currentLevel * HashmapSize;
        levelInfo.scale = baseScale * exp2(float(currentLevel) * log2PerLevelScale);

        // Initialize RNG for random shifts (different levels should draw differently)
        Pcg32 rng = Pcg32(seed);
        rng.advance(int64_t(currentLevel * Dimensions));

        [ForceUnroll]
        for (uint dim = 0; dim < Dimensions; ++dim)
        {
            levelInfo.scalesPerDim[dim] = levelInfo.scale * rsqrt(float((dim + 1) * (dim + 2)));
            // Convert uint32 to float in [0, 1) then scale to [-5, 5)
            float randFloat = float(rng.nextUint()) * (1.0f / 4294967296.0f);
            levelInfo.shiftsPerDim[dim] = randFloat * 10.0f - 5.0f;
        }
    }
}
