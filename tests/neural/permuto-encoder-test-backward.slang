// Test permuto encoder backward pass using autodiff

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type -xslang -experimental-feature -emit-spirv-directly

import neural;
import "permuto-reference-data";

// Test configuration (must match CUDA reference permuto_jit_test.cu)
static const int N_POS_DIMS = 2;
static const int FEATURE_DIM_PER_ENTRY = 2;
static const uint N_LEVELS = 4;
static const uint N_FEATURES = N_LEVELS * FEATURE_DIM_PER_ENTRY;  // 8
static const uint GRID_SIZE = 16;  // 16x16 positions
static const uint N_POSITIONS = GRID_SIZE * GRID_SIZE;  // 256

// Hash table size: 2^LOG2_HASHMAP_SIZE per level (matching PermutoEncodingTemplated)
static const uint LOG2_HASHMAP_SIZE = 8;
static const uint HASHMAP_SIZE = 1u << LOG2_HASHMAP_SIZE;  // 256 entries per level
static const uint TOTAL_TABLE_SIZE = HASHMAP_SIZE * N_LEVELS;  // 1024 entries total
static const uint FEATURE_TABLE_SIZE = TOTAL_TABLE_SIZE * FEATURE_DIM_PER_ENTRY;  // 2048 floats

// Encoder configuration
static const float BASE_SCALE = 4.0f;
static const float LOG2_PER_LEVEL_SCALE = 1.5f;
static const float MAX_LEVEL_RATIO = 1.1f;  // Matches tiny-cuda-nn default

// Type alias for the encoder
typealias InArray = float[N_POS_DIMS];
typealias OutArray = float[N_FEATURES];
typealias Encoder = PermutoEncoder<float, N_POS_DIMS, FEATURE_DIM_PER_ENTRY, LOG2_HASHMAP_SIZE, N_LEVELS, InArray, OutArray>;

static const float EPSILON = 0.1f;  // Allow for floating-point precision differences

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

/// Differentiable forward pass that encodes position through all levels
/// Returns the concatenated output features from all levels
[Differentiable]
void encodeAllLevels(
    float position[N_POS_DIMS],
    BindlessAddress<float> featureAddress,
    out float output[N_FEATURES])
{
    Encoder.Params params;
    params.maxLevel = MAX_LEVEL_RATIO * float(N_LEVELS);

    // Process all levels - encodePerLevel writes to output at correct offset
    [ForceUnroll]
    for (uint level = 0; level < N_LEVELS; ++level)
    {
        // Setup params using prepareLevelInfo which computes all level-specific parameters
        Encoder.prepareLevelInfo(level, BASE_SCALE, LOG2_PER_LEVEL_SCALE, 1337u, params.levelInfo);

        // Encode this level - writes to output at offset level * FEATURE_DIM_PER_ENTRY
        Encoder.encodePerLevel<BindlessAddress<float>>(
            params, position, featureAddress, output);
    }
}

/// Compute backward pass for all levels by calling bwd_diff on encodeAllLevels
void encodeBackwardAllLevels(
    float position[N_POS_DIMS],
    BindlessAddress<float> featureAddress,
    BindlessAddress<float> dfeatureAddress,
    out float dL_dpos[N_POS_DIMS])
{
    // Create differential pair for position
    float dPosition[N_POS_DIMS] = {0.0f, 0.0f};
    var dpPosition = diffPair(position, dPosition);

    // Create differential pair for feature address
    var dpFeatureAddress = DifferentialPtrPair<BindlessAddress<float>>(featureAddress, dfeatureAddress);

    // Upstream gradient: all ones for all output features
    float dL_dy[N_FEATURES];
    [ForceUnroll]
    for (uint i = 0; i < N_FEATURES; ++i)
    {
        dL_dy[i] = 1.0f;
    }

    // Call backward pass on the whole encodeAllLevels function
    bwd_diff(encodeAllLevels)(dpPosition, dpFeatureAddress, dL_dy);

    // Extract position gradient
    dL_dpos[0] = dpPosition.d[0];
    dL_dpos[1] = dpPosition.d[1];
}

/// Verify dL/dpos for a position against expected values
bool verifyPosition(uint verifyIdx, uint posIdx)
{
    [ForceUnroll]
    for (uint dim = 0; dim < N_POS_DIMS; ++dim)
    {
        uint outIdx = posIdx * N_POS_DIMS + dim;
        uint expectedIdx = verifyIdx * N_POS_DIMS + dim;
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

[shader("compute")]
[numthreads(256, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint posIdx = dispatchThreadID.x;
    if (posIdx >= N_POSITIONS) return;

    // Initialize feature table: value[i] = i
    // Each thread handles 8 values
    uint baseIdx = posIdx * 8;
    [ForceUnroll]
    for (uint i = 0; i < 8; ++i)
    {
        uint idx = baseIdx + i;
        if (idx < FEATURE_TABLE_SIZE)
        {
            gFeatureTable[idx] = idx;
            gDFeatureTable[idx] = 0.0f;  // Initialize gradient to zero
        }
    }

    // Barrier to ensure all threads have finished initializing
    AllMemoryBarrierWithGroupSync();

    // Compute position: center of grid cell in [0, 1]
    uint x = posIdx % GRID_SIZE;
    uint y = posIdx / GRID_SIZE;
    float position[N_POS_DIMS];
    position[0] = (float(x) + 0.5f) / float(GRID_SIZE);
    position[1] = (float(y) + 0.5f) / float(GRID_SIZE);

    // Create addresses for feature table and its gradient
    BindlessAddress<float> featureAddress = BindlessAddress<float>(gFeatureTable);
    BindlessAddress<float> dfeatureAddress = BindlessAddress<float>(gDFeatureTable);

    // Compute backward pass for all levels
    float dL_dpos[N_POS_DIMS];
    encodeBackwardAllLevels(position, featureAddress, dfeatureAddress, dL_dpos);

    // Write dL/dpos to buffer
    gDLdPosBuffer[posIdx * N_POS_DIMS + 0] = dL_dpos[0];
    gDLdPosBuffer[posIdx * N_POS_DIMS + 1] = dL_dpos[1];

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
