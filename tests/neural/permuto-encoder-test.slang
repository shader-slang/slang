// Test permuto encoder forward pass
// Reference: tiny-cuda-nn permuto.h encoding
// Run the CUDA reference first: cd tiny-cuda-nn/build && ./permuto_jit_test

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type -xslang -experimental-feature

import neural;

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

// Type alias for the encoder
typealias Encoder = PermutoEncoder<float, N_POS_DIMS, FEATURE_DIM_PER_ENTRY, LOG2_HASHMAP_SIZE>;

// ============================================================================
// Verification data from CUDA reference (permuto_jit_test.cu)
// ============================================================================
static const uint NUM_VERIFY_POSITIONS = 16;
static const uint VERIFY_POSITIONS[NUM_VERIFY_POSITIONS] = {
    0, 5, 17, 42, 63, 100, 128, 150,
    180, 199, 210, 230, 240, 250, 253, 255
};

// Expected values: flattened [NUM_VERIFY_POSITIONS * N_FEATURES] = [128]
// Layout: [pos0_level0_f0, pos0_level0_f1, ..., pos0_level3_f1, pos1_level0_f0, ...]
static const float EXPECTED_VALUES[NUM_VERIFY_POSITIONS * N_FEATURES] = {
    // pos=0
    273.577118f, 274.577148f, 614.023804f, 615.023804f, 1117.943237f, 1118.943237f, 1754.296875f, 1755.296875f,
    // pos=5
    171.046661f, 172.046661f, 750.690491f, 751.690430f, 1504.494873f, 1505.494873f, 1715.986938f, 1716.986816f,
    // pos=17
    216.328766f, 217.328751f, 685.235718f, 686.235657f, 1319.360962f, 1320.360962f, 1704.348511f, 1705.348511f,
    // pos=42
    35.614384f, 36.614384f, 643.529175f, 644.529175f, 1189.797974f, 1190.797974f, 1975.376099f, 1976.376221f,
    // pos=63
    99.446777f, 100.446777f, 643.659668f, 644.659729f, 1113.920776f, 1114.920776f, 1883.739502f, 1884.739380f,
    // pos=100
    39.685390f, 40.685390f, 986.628906f, 987.628967f, 1147.829468f, 1148.829468f, 1565.992920f, 1566.992920f,
    // pos=128
    32.057064f, 33.057068f, 965.052917f, 966.052917f, 1086.802979f, 1087.802979f, 1894.884521f, 1895.884521f,
    // pos=150
    105.553284f, 106.553284f, 660.931458f, 661.931458f, 1162.772461f, 1163.772461f, 1727.836426f, 1728.836426f,
    // pos=180
    117.252563f, 118.252563f, 694.021973f, 695.021973f, 1256.366821f, 1257.366821f, 1774.636475f, 1775.636475f,
    // pos=199
    161.757385f, 162.757385f, 819.900513f, 820.900513f, 1302.556396f, 1303.556396f, 1757.666504f, 1758.666504f,
    // pos=210
    128.951828f, 129.951828f, 727.112610f, 728.112610f, 1349.960938f, 1350.961060f, 1794.304199f, 1795.304199f,
    // pos=230
    183.120438f, 184.120438f, 880.324524f, 881.324524f, 1271.310059f, 1272.309937f, 1729.058105f, 1730.057983f,
    // pos=240
    140.651123f, 141.651123f, 760.203247f, 761.203186f, 1443.555420f, 1444.555542f, 1792.086548f, 1793.086670f,
    // pos=250
    237.289078f, 238.289078f, 828.529419f, 829.529419f, 1192.658936f, 1193.658936f, 1930.753052f, 1931.753174f,
    // pos=253
    266.280457f, 267.280426f, 654.529358f, 655.529358f, 1424.589844f, 1425.589966f, 1683.534424f, 1684.534424f,
    // pos=255
    285.608032f, 286.608032f, 658.203247f, 659.203247f, 1156.350098f, 1157.349976f, 1779.534546f, 1780.534546f
};

static const float EPSILON = 0.1f;  // Allow for floating-point precision differences between Slang and CUDA

// ============================================================================
// Buffers
// ============================================================================

// Feature table buffer: will be initialized by threads with value[i] = i
// Total size: 1024 * 2 = 2048 floats
//TEST_INPUT:ubuffer(count=2048, stride=4):name=gFeatureTable
RWStructuredBuffer<float> gFeatureTable;

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
            gFeatureTable[idx] = float(idx);
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
        float expected = EXPECTED_VALUES[expectedIdx];
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

    // Create storage for feature table
    StructuredBufferStorage<float> featureStorage = StructuredBufferStorage<float>(gFeatureTable);

    // Process each level
    [ForceUnroll]
    for (uint level = 0; level < N_LEVELS; ++level)
    {
        // Compute per-dimension scales and shifts using helper function
        float scalesPerDim[N_POS_DIMS];
        float shiftsPerDim[N_POS_DIMS];
        Encoder.prepareShiftAndScalePerDim(BASE_SCALE, level, LOG2_PER_LEVEL_SCALE, 1337u, scalesPerDim, shiftsPerDim);

        // Setup params
        Encoder.Params params;
        params.currentLevel = level;
        params.maxLevel = float(N_LEVELS);
        params.scalesPerDim = scalesPerDim;
        params.shiftsPerDim = shiftsPerDim;

        // Setup level info using helper function
        Encoder.prepareLevelInfo(level, params.levelInfo);

        // Encode
        float result[FEATURE_DIM_PER_ENTRY] = Encoder.encode(
            params,
            position,
            featureStorage,
            0u  // base address
        );

        // Write output
        [ForceUnroll]
        for (uint f = 0; f < FEATURE_DIM_PER_ENTRY; ++f)
        {
            uint outIdx = posIdx * N_FEATURES + level * FEATURE_DIM_PER_ENTRY + f;
            gOutputBuffer[outIdx] = result[f];
        }
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
