// Test permuto encoder backward pass using the ITrainableEncoder interface
// This test verifies backward differentiation through Encoder.encode() with HyperParameters
//
// Reference: tiny-cuda-nn permuto.h encoding
// Run the CUDA reference first: cd tiny-cuda-nn/build && ./permuto_jit_test b

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type -xslang -experimental-feature

import neural;
import "permuto-encoder-precomputed-constants";
import "permuto-reference-data";

static const float EPSILON = 0.1f;  // Allow for floating-point precision differences

// Type aliases using precomputed constants
typealias InArray = InlineVector<float, lc_nPosDims>;
typealias OutArray = InlineVector<float, lc_nFeatures>;
typealias Encoder = PermutoEncoder<float, lc_nPosDims, lc_featureDimPerEntry, lc_log2HashmapSize, lc_nLevels, InArray, OutArray>;

// ============================================================================
// Build HyperParameters from precomputed flat arrays
// ============================================================================

Encoder.HyperParameters buildHyperParameters()
{
    Encoder.HyperParameters params;
    params.maxLevel = lc_maxLevel;

    [ForceUnroll]
    for (uint level = 0; level < lc_nLevels; ++level)
    {
        params.levelInfo[level].currentLevelIndex = lc_levelIndices[level];
        params.levelInfo[level].offsetOfFeatureTable = lc_offsets[level];
        params.levelInfo[level].featureTableSize = lc_tableSizes[level];
        params.levelInfo[level].scale = lc_scales[level];

        [ForceUnroll]
        for (uint dim = 0; dim < lc_nPosDims; ++dim)
        {
            params.levelInfo[level].scalesPerDim[dim] = lc_scalesPerDim[level * lc_nPosDims + dim];
            params.levelInfo[level].shiftsPerDim[dim] = lc_shiftsPerDim[level * lc_nPosDims + dim];
        }
    }

    return params;
}

// ============================================================================
// Buffers
// ============================================================================

// Feature table buffer: will be initialized by threads with value[i] = i
// Total size: 1024 * 2 = 2048 floats
//TEST_INPUT: set gFeatureTable = ubuffer(count=2048, stride=4)
uniform RWStructuredBuffer<float>.Handle gFeatureTable;

// Gradient buffer for feature table (dL/dparams)
//TEST_INPUT: set gDFeatureTable = ubuffer(count=2048, stride=4)
uniform RWStructuredBuffer<float>.Handle gDFeatureTable;

// Output buffer for dL/dpos: N_POSITIONS * N_POS_DIMS = 512 floats
//TEST_INPUT:ubuffer(count=512, stride=4):name=gDLdPosBuffer
RWStructuredBuffer<float> gDLdPosBuffer;

// Verification result buffer: [0] = position pass count, [1] = param pass count
//TEST_INPUT:ubuffer(count=2, stride=4):out,name=gVerifyBuffer
RWStructuredBuffer<uint> gVerifyBuffer;

// ============================================================================
// Differentiable Forward Pass using Interface
// ============================================================================

/// Differentiable forward pass using Encoder.encode with HyperParameters
[Differentiable]
OutArray encodeWithInterface(
    InArray position,
    Encoder.HyperParameters hyperParams,
    BindlessAddress<float> featureAddress)
{
    Encoder encoder = Encoder(hyperParams);
    return encoder.encode<BindlessAddress<float>>(position, featureAddress);
}

/// Compute backward pass by calling bwd_diff on encodeWithInterface
void encodeBackward(
    InArray position,
    Encoder.HyperParameters hyperParams,
    BindlessAddress<float> featureAddress,
    BindlessAddress<float> dfeatureAddress,
    out InArray dL_dpos)
{
    // Create differential pair for position
    InArray dPosition = InArray();
    var dpPosition = diffPair(position, dPosition);

    // Create differential pair for feature address
    var dpFeatureAddress = DifferentialPtrPair<BindlessAddress<float>>(featureAddress, dfeatureAddress);

    // Upstream gradient: all ones for all output features
    OutArray dL_dy = OutArray(1.0f);

    // Call backward pass on encodeWithInterface
    bwd_diff(encodeWithInterface)(dpPosition, hyperParams, dpFeatureAddress, dL_dy);

    // Extract position gradient
    dL_dpos = dpPosition.d;
}

// ============================================================================
// Verification Functions
// ============================================================================

/// Verify dL/dpos for a position against expected values
bool verifyPosition(uint verifyIdx, uint posIdx)
{
    [ForceUnroll]
    for (uint dim = 0; dim < lc_nPosDims; ++dim)
    {
        uint outIdx = posIdx * lc_nPosDims + dim;
        uint expectedIdx = verifyIdx * lc_nPosDims + dim;
        float expected = EXPECTED_DL_DPOS[expectedIdx];
        float actual = gDLdPosBuffer[outIdx];
        float diff = abs(actual - expected);
        if (diff > EPSILON)
        {
            return false;
        }
    }
    return true;
}

/// Verify dL/dparams (feature table gradients) against expected values
bool verifyParam(uint verifyIdx)
{
    uint paramIdx = VERIFY_PARAM_INDICES[verifyIdx];
    float expected = EXPECTED_DL_DPARAMS[verifyIdx];
    float actual = gDFeatureTable[paramIdx];
    float diff = abs(actual - expected);
    return diff <= EPSILON;
}

// ============================================================================
// Main Compute Shader
// ============================================================================

[shader("compute")]
[numthreads(256, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint posIdx = dispatchThreadID.x;
    if (posIdx >= lc_nPositions) return;

    // Initialize feature table: value[i] = i
    // Each thread handles 8 values
    uint baseIdx = posIdx * 8;
    [ForceUnroll]
    for (uint i = 0; i < 8; ++i)
    {
        uint idx = baseIdx + i;
        if (idx < lc_featureTableSize)
        {
            gFeatureTable[idx] = idx;
            gDFeatureTable[idx] = 0.0f;  // Initialize gradient to zero
        }
    }

    // Barrier to ensure all threads have finished initializing
    AllMemoryBarrierWithGroupSync();

    // Compute position: center of grid cell in [0, 1]
    uint x = posIdx % lc_gridSize;
    uint y = posIdx / lc_gridSize;
    InArray position;
    position[0] = (float(x) + 0.5f) / float(lc_gridSize);
    position[1] = (float(y) + 0.5f) / float(lc_gridSize);

    // Create addresses for feature table and its gradient
    BindlessAddress<float> featureAddress = BindlessAddress<float>(gFeatureTable);
    BindlessAddress<float> dfeatureAddress = BindlessAddress<float>(gDFeatureTable);

    // Build HyperParameters from precomputed flat arrays
    Encoder.HyperParameters hyperParams = buildHyperParameters();

    // Compute backward pass using the interface
    InArray dL_dpos;
    encodeBackward(position, hyperParams, featureAddress, dfeatureAddress, dL_dpos);

    // Write dL/dpos to buffer
    gDLdPosBuffer[posIdx * lc_nPosDims + 0] = dL_dpos[0];
    gDLdPosBuffer[posIdx * lc_nPosDims + 1] = dL_dpos[1];

    // Barrier to ensure all threads have finished writing results
    AllMemoryBarrierWithGroupSync();

    // Only thread 0 does the verification
    if (posIdx != 0) return;

    // Verify the 16 selected positions for dL/dpos
    uint posPassCount = 0;
    [ForceUnroll]
    for (uint i = 0; i < NUM_VERIFY_POSITIONS; ++i)
    {
        if (verifyPosition(i, VERIFY_POSITIONS[i]))
        {
            posPassCount++;
        }
    }

    // Verify the 64 selected feature table gradients
    uint paramPassCount = 0;
    [ForceUnroll]
    for (uint i = 0; i < NUM_VERIFY_PARAMS; ++i)
    {
        if (verifyParam(i))
        {
            paramPassCount++;
        }
    }

    // Output: [0] = position pass count (expect 16), [1] = param pass count (expect 64)
    gVerifyBuffer[0] = posPassCount;
    gVerifyBuffer[1] = paramPassCount;
}

// Expected output: all 16 position verifications and all 64 param verifications should pass
// BUF: 16
// BUF-NEXT: 64
