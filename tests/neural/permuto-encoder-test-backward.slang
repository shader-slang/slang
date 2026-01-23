// Test permuto encoder backward pass using autodiff
// Reference: tiny-cuda-nn permuto.h encoding
// Run the CUDA reference first: cd tiny-cuda-nn/build && ./permuto_reference b

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature

import neural;

// Test configuration (must match CUDA reference permuto_reference.cu)
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
// Buffers
// ============================================================================

// Feature table buffer: will be initialized by threads with value[i] = i
// Total size: 1024 * 2 = 2048 floats
//TEST_INPUT:ubuffer(count=2048, stride=4):name=gFeatureTable
RWStructuredBuffer<float> gFeatureTable;

// Gradient buffer for feature table (dL/dparams)
//TEST_INPUT:ubuffer(count=2048, stride=4):name=gDFeatureTable
RWStructuredBuffer<float> gDFeatureTable;

// Output buffer for dL/dpos: N_POSITIONS * N_POS_DIMS = 512 floats
//TEST_INPUT:ubuffer(count=512, stride=4):name=gDLdPosBuffer
RWStructuredBuffer<float> gDLdPosBuffer;

// Simple output buffer to verify the test ran
//TEST_INPUT:ubuffer(count=1, stride=4):out,name=gResultBuffer
RWStructuredBuffer<uint> gResultBuffer;

/// Compute backward pass for all levels using bwd_diff
void encodeBackwardAllLevels(
    float position[N_POS_DIMS],
    StructuredBufferStorage<float> featureStorage,
    StructuredBufferStorage<float> dfeatureStorage,
    out float dL_dpos[N_POS_DIMS])
{
    // Initialize output gradient to zero
    dL_dpos[0] = 0.0f;
    dL_dpos[1] = 0.0f;

    // Process all levels
    [ForceUnroll]
    for (uint level = 0; level < N_LEVELS; ++level)
    {
        // Compute per-dimension scales and shifts
        float scalesPerDim[N_POS_DIMS];
        float shiftsPerDim[N_POS_DIMS];
        Encoder.prepareShiftAndScalePerDim(BASE_SCALE, level, LOG2_PER_LEVEL_SCALE, 1337u, scalesPerDim, shiftsPerDim);

        // Setup params
        Encoder.Params params;
        params.currentLevel = level;
        params.maxLevel = float(N_LEVELS);
        params.scalesPerDim = scalesPerDim;
        params.shiftsPerDim = shiftsPerDim;
        Encoder.prepareLevelInfo(level, params.levelInfo);

        // Create differential pairs for backward pass
        float dPosition[N_POS_DIMS] = {0.0f, 0.0f};
        var dpPosition = diffPair(position, dPosition);

        // Create differential pair for feature storage
        var dpFeatureStorage = DifferentialPtrPair<StructuredBufferStorage<float>>(featureStorage, dfeatureStorage);

        // Upstream gradient: all ones
        float dL_dy[FEATURE_DIM_PER_ENTRY] = {1.0f, 1.0f};

        // Call backward pass
        bwd_diff(Encoder.encode)(params, dpPosition, dpFeatureStorage, 0u, dL_dy);

        // Accumulate position gradient
        dL_dpos[0] += dpPosition.d[0];
        dL_dpos[1] += dpPosition.d[1];
    }
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
            gFeatureTable[idx] = float(idx);
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

    // Create storage for feature table and its gradient
    StructuredBufferStorage<float> featureStorage = StructuredBufferStorage<float>(gFeatureTable);
    StructuredBufferStorage<float> dfeatureStorage = StructuredBufferStorage<float>(gDFeatureTable);

    // Compute backward pass for all levels
    float dL_dpos[N_POS_DIMS];
    encodeBackwardAllLevels(position, featureStorage, dfeatureStorage, dL_dpos);

    // Write dL/dpos to buffer
    gDLdPosBuffer[posIdx * N_POS_DIMS + 0] = dL_dpos[0];
    gDLdPosBuffer[posIdx * N_POS_DIMS + 1] = dL_dpos[1];

    // Thread 0 writes a marker to verify test ran
    if (posIdx == 0)
    {
        gResultBuffer[0] = 1u;
    }
}

// Verify the test ran successfully
// BUF: 1
