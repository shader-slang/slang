// Test permuto encoder forward pass
// Reference: tiny-cuda-nn permuto.h encoding
// Run the CUDA reference first: cd tiny-cuda-nn/tests && nvcc -arch=native --expt-relaxed-constexpr -I../include -I../dependencies -o permuto_reference permuto_reference.cu && ./permuto_reference

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type

import neural;

// Test configuration (must match CUDA reference)
static const int N_POS_DIMS = 2;
static const int N_FEATURES_PER_LEVEL = 2;
static const uint N_LEVELS = 2;
static const uint GRID_SIZE = 16;  // 16x16 positions
static const uint N_POSITIONS = GRID_SIZE * GRID_SIZE;  // 256

// Hash table sizes: ceil(scale)^2 for each level (used in levelInfo setup)
// Level 0: scale=4, table_size=4^2=16
// Level 1: scale=4*2^1.5≈11.31, table_size=ceil(11.31)^2=12^2=144
static const uint TABLE_SIZE_L0 = 16;
static const uint TABLE_SIZE_L1 = 144;
static const uint TOTAL_TABLE_SIZE = TABLE_SIZE_L0 + TABLE_SIZE_L1;  // 160
static const uint FEATURE_TABLE_SIZE = TOTAL_TABLE_SIZE * N_FEATURES_PER_LEVEL;  // 320

// Encoder configuration
static const float BASE_SCALE = 4.0f;
static const float LOG2_PER_LEVEL_SCALE = 1.5f;

// Type alias for the encoder
typealias Encoder = PermutoEncoder<float, N_POS_DIMS, N_FEATURES_PER_LEVEL, N_LEVELS * N_FEATURES_PER_LEVEL, 16>;

// Feature table buffer: will be initialized by threads with value[i] = i
// Total size: (16 + 144) * 2 = 320 floats
//TEST_INPUT:ubuffer(count=320, stride=4):name=gFeatureTable
RWStructuredBuffer<float> gFeatureTable;

/// Initialize feature table: each thread fills part of the table with value[i] = i
void initFeatureTable(uint threadIdx)
{
    // 256 threads, 320 values -> each thread handles ceil(320/256) = 2 values
    // Thread i handles indices [i*2, i*2+1] if within bounds
    uint baseIdx = threadIdx * 2;
    if (baseIdx < FEATURE_TABLE_SIZE)
    {
        gFeatureTable[baseIdx] = float(baseIdx);
    }
    if (baseIdx + 1 < FEATURE_TABLE_SIZE)
    {
        gFeatureTable[baseIdx + 1] = float(baseIdx + 1);
    }
}

// Output buffer: N_POSITIONS * N_LEVELS * N_FEATURES_PER_LEVEL = 256 * 2 * 2 = 1024 floats
//TEST_INPUT:ubuffer(count=1024, stride=4):out,name=gOutputBuffer
RWStructuredBuffer<float> gOutputBuffer;

// Test positions: 16x16 = 256
// static const uint N_TEST_POSITIONS = 256;

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
        // Compute scale for this level
        float scale = BASE_SCALE * exp2(float(level) * LOG2_PER_LEVEL_SCALE);

        // Compute per-dimension scales and shifts
        float scalesPerDim[N_POS_DIMS];
        float shiftsPerDim[N_POS_DIMS];

        // Use fixed seed for reproducibility (same as CUDA reference)
        Pcg32 rng = Pcg32(1337);
        rng.advance(int64_t(level * N_POS_DIMS));

        [ForceUnroll]
        for (uint dim = 0; dim < N_POS_DIMS; ++dim)
        {
            scalesPerDim[dim] = scale * rsqrt(float((dim + 1) * (dim + 2)));
            // Convert uint32 to float in [0, 1) then scale to [-5, 5)
            float randFloat = float(rng.nextUint()) * (1.0f / 4294967296.0f);
            shiftsPerDim[dim] = randFloat * 10.0f - 5.0f;
        }

        // Setup params
        Encoder.Params params;
        params.currentLevel = level;
        params.maxLevel = float(N_LEVELS);
        params.baseScale = BASE_SCALE;
        params.log2PerLevelScale = LOG2_PER_LEVEL_SCALE;

        // Setup level info
        if (level == 0) {
            params.levelInfo.offset = 0;
            params.levelInfo.hashmapSize = TABLE_SIZE_L0;
        } else {
            params.levelInfo.offset = TABLE_SIZE_L0;
            params.levelInfo.hashmapSize = TABLE_SIZE_L1;
        }

        // Encode
        float result[N_FEATURES_PER_LEVEL] = Encoder.encode(
            params,
            position,
            scalesPerDim,
            shiftsPerDim,
            featureStorage,
            0u  // base address
        );

        // Write output
        [ForceUnroll]
        for (uint f = 0; f < N_FEATURES_PER_LEVEL; ++f)
        {
            uint outIdx = posIdx * N_LEVELS * N_FEATURES_PER_LEVEL + level * N_FEATURES_PER_LEVEL + f;
            gOutputBuffer[outIdx] = result[f];
        }
    }
}

// Expected values will be generated by running permuto_reference.cu
// For now, just checking that it compiles and runs
// BUF: {{.*}}
