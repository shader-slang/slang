// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// For-loop (scalar) benchmark - matches original SlangPy LinearLayer style.
// Network architecture configurable via preprocessor defines.
// This is the scalar baseline running on GPU without tensor core acceleration.

// Network configuration - use defines or defaults
#ifndef INPUT_SIZE
#define INPUT_SIZE 64
#endif
#ifndef HIDDEN_SIZE
#define HIDDEN_SIZE 64
#endif
#ifndef OUTPUT_SIZE
#define OUTPUT_SIZE 16
#endif

// Parameter offsets in flat buffer
static const int LAYER1_WEIGHT_OFFSET = 0;
static const int LAYER1_BIAS_OFFSET = INPUT_SIZE * HIDDEN_SIZE;
static const int LAYER2_WEIGHT_OFFSET = LAYER1_BIAS_OFFSET + HIDDEN_SIZE;
static const int LAYER2_BIAS_OFFSET = LAYER2_WEIGHT_OFFSET + HIDDEN_SIZE * OUTPUT_SIZE;

// Linear layer forward pass using for-loops (matches original LinearLayer)
void linearLayerForward<let NumInputs : int, let NumOutputs : int>(
    StructuredBuffer<float> weights,
    int weightOffset,
    StructuredBuffer<float> biases, 
    int biasOffset,
    float input[NumInputs],
    out float output[NumOutputs])
{
    for (int row = 0; row < NumOutputs; ++row)
    {
        float sum = biases[biasOffset + row];
        [ForceUnroll]
        for (int col = 0; col < NumInputs; ++col)
        {
            sum += weights[weightOffset + row * NumInputs + col] * input[col];
        }
        output[row] = sum;
    }
}

// LeakyReLU activation
void leakyReLU<let N : int>(inout float x[N])
{
    [ForceUnroll]
    for (int i = 0; i < N; ++i)
    {
        if (x[i] < 0.0f)
            x[i] *= 0.01f;
    }
}

// Query functions
public int get_input_size() { return INPUT_SIZE; }
public int get_hidden_size() { return HIDDEN_SIZE; }
public int get_output_size() { return OUTPUT_SIZE; }

// Compute shader entry point - one thread per sample
[shader("compute")]
[numthreads(64, 1, 1)]
void compute_batch_forward(
    uint3 tid : SV_DispatchThreadID,
    StructuredBuffer<float> params,
    StructuredBuffer<float> inputs,
    RWStructuredBuffer<float> outputs,
    uniform int batch_size)
{
    int sampleIdx = tid.x;
    if (sampleIdx >= batch_size)
        return;
    
    // Load input
    float input[INPUT_SIZE];
    int inBaseIdx = sampleIdx * INPUT_SIZE;
    [ForceUnroll]
    for (int i = 0; i < INPUT_SIZE; ++i)
    {
        input[i] = inputs[inBaseIdx + i];
    }
    
    // Layer 1: Input -> Hidden
    float hidden[HIDDEN_SIZE];
    linearLayerForward<INPUT_SIZE, HIDDEN_SIZE>(
        params, LAYER1_WEIGHT_OFFSET,
        params, LAYER1_BIAS_OFFSET,
        input, hidden);
    leakyReLU<HIDDEN_SIZE>(hidden);
    
    // Layer 2: Hidden -> Output
    float output[OUTPUT_SIZE];
    linearLayerForward<HIDDEN_SIZE, OUTPUT_SIZE>(
        params, LAYER2_WEIGHT_OFFSET,
        params, LAYER2_BIAS_OFFSET,
        hidden, output);
    leakyReLU<OUTPUT_SIZE>(output);
    
    // Write output
    int outBaseIdx = sampleIdx * OUTPUT_SIZE;
    [ForceUnroll]
    for (int i = 0; i < OUTPUT_SIZE; ++i)
    {
        outputs[outBaseIdx + i] = output[i];
    }
}
