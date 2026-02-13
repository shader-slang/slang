// Test permuto encoder forward pass

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type -xslang -experimental-feature

import neural;
import "permuto-reference-data";

// Test configuration (must match CUDA reference permuto_reference.cu and permuto_jit_test.cu)
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

static const float EPSILON = 0.1f;  // Allow for floating-point precision differences between Slang and CUDA

// ============================================================================
// Buffers
// ============================================================================

// Feature table buffer: will be initialized by threads with value[i] = i
// Total size: 1024 * 2 = 2048 floats
//TEST_INPUT: set gFeatureTable = ubuffer(count=2048, stride=4)
uniform RWStructuredBuffer<float>.Handle gFeatureTable;

// Output buffer: store all results (N_POSITIONS * N_FEATURES = 2048 floats)
//TEST_INPUT:ubuffer(count=2048, stride=4):name=gOutputBuffer
RWStructuredBuffer<float> gOutputBuffer;

// Verification result buffer: pass count
//TEST_INPUT:ubuffer(count=1, stride=4):out,name=gVerifyBuffer
RWStructuredBuffer<uint> gVerifyBuffer;

/// Initialize feature table: each thread fills part of the table with value[i] = i
void initFeatureTable(uint threadIdx)
{
    // 256 threads, 2048 values -> each thread handles 8 values
    // Thread i handles indices [i*8, i*8+1, ..., i*8+7]
    uint baseIdx = threadIdx * 8;
    [ForceUnroll]
    for (uint i = 0; i < 8; ++i)
    {
        uint idx = baseIdx + i;
        if (idx < FEATURE_TABLE_SIZE)
        {
            gFeatureTable[idx] = idx;
        }
    }
}

/// Check if a position index is one of the verification positions
int getVerifyIndex(uint posIdx)
{
    [ForceUnroll]
    for (uint i = 0; i < NUM_VERIFY_POSITIONS; ++i)
    {
        if (VERIFY_POSITIONS[i] == posIdx)
        {
            return int(i);
        }
    }
    return -1;
}

/// Verify results for a position against expected values
bool verifyPosition(uint verifyIdx, uint posIdx)
{
    // Compare with expected values
    [ForceUnroll]
    for (uint f = 0; f < N_FEATURES; ++f)
    {
        uint outIdx = posIdx * N_FEATURES + f;
        uint expectedIdx = verifyIdx * N_FEATURES + f;
        float expected = EXPECTED_FORWARD_VALUES[expectedIdx];
        float actual = gOutputBuffer[outIdx];
        float diff = abs(actual - expected);
        if (diff > EPSILON)
        {
            return false;
        }
    }
    return true;
}

[shader("compute")]
[numthreads(256, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint posIdx = dispatchThreadID.x;

    // Initialize feature table: each thread fills part of the table with value[i] = i
    initFeatureTable(posIdx);

    // Barrier to ensure all threads have finished initializing the feature table
    AllMemoryBarrierWithGroupSync();

    if (posIdx >= N_POSITIONS) return;

    // Compute position: center of grid cell in [0, 1]
    uint x = posIdx % GRID_SIZE;
    uint y = posIdx / GRID_SIZE;
    float position[N_POS_DIMS];
    position[0] = (float(x) + 0.5f) / float(GRID_SIZE);
    position[1] = (float(y) + 0.5f) / float(GRID_SIZE);

    // Create address for feature table
    BindlessAddress<float> featureAddress = BindlessAddress<float>(gFeatureTable);

    // Initialize output array
    OutArray result;

    // Process each level
    [ForceUnroll]
    for (uint level = 0; level < N_LEVELS; ++level)
    {
        // Setup params using prepareLevelInfo which computes all level-specific parameters
        Encoder.Params params;
        params.maxLevel = MAX_LEVEL_RATIO * float(N_LEVELS);
        Encoder.prepareLevelInfo(level, BASE_SCALE, LOG2_PER_LEVEL_SCALE, 1337u, params.levelInfo);

        // Encode - writes to result at offset level * FEATURE_DIM_PER_ENTRY
        Encoder.encodePerLevel<BindlessAddress<float>>(
            params,
            position,
            featureAddress,
            result
        );
    }

    // Write output
    [ForceUnroll]
    for (uint f = 0; f < N_FEATURES; ++f)
    {
        gOutputBuffer[posIdx * N_FEATURES + f] = result[f];
    }

    // Barrier to ensure all threads have finished writing results
    AllMemoryBarrierWithGroupSync();

    // Only thread 0 does the verification
    if (posIdx != 0) return;

    // Verify the 16 selected positions
    uint passCount = 0;
    [ForceUnroll]
    for (uint i = 0; i < NUM_VERIFY_POSITIONS; ++i)
    {
        if (verifyPosition(i, VERIFY_POSITIONS[i]))
        {
            passCount++;
        }
    }

    // Output: 16 means all passed
    gVerifyBuffer[0] = passCount;
}

// Expected output: all 16 verification positions should pass
// BUF: 16
