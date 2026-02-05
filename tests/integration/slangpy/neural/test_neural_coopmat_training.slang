// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// SlangPy test for FFLayer with CoopMat (WaveTangledVector) backend.
// This extends test_neural_frontend_training.slang to cover the cooperative matrix backend.
//
// Key requirements for CoopMat:
// - Must use explicit compute shader entry points with [numthreads(32, 1, 1)]
// - Types must be defined inside shader functions
// - Requires cooperative matrix capabilities

import neural;

typealias Storage = StructuredBufferStorage<float>;

// Shared memory configuration
static const int InputSize = 2;
static const int OutputSize = 1;
static const int SubgroupSize = 32;
static const int BatchSize = 32;

typealias ShMemSize = SharedMemorySize<float, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, BatchSize / SubgroupSize>;
typealias ShMemSizeLayer = ShMemSize.OfLayer1<InputSize, OutputSize>;

// Linear layer: Input=2 (x, x^2), Output=1 (y), with bias
// Parameters: weights (1x2) + bias (1) = 3 params
static const int PARAM_COUNT = 3;

// Simple function to get param count (doesn't need CoopMat)
int get_param_count()
{
    return PARAM_COUNT;
}

// Compute shader entry point for forward pass test
// Dispatched with [1, 1, 1] groups, 32 threads per group
[shader("compute")]
[numthreads(32, 1, 1)]
void compute_forward_pass(
    uint3 tid : SV_DispatchThreadID,
    uint gtid : SV_GroupIndex,
    RWStructuredBuffer<float> params,
    RWStructuredBuffer<float> result,
    uniform float x0,
    uniform float x1)
{
    // Define CoopMat types inside shader function
    typealias ShMemPool = SharedMemoryPool<ShMemSizeLayer>;
    typealias V2 = WaveTangledVector<float, ShMemPool, InputSize, SubgroupSize>;
    typealias V1 = WaveTangledVector<float, ShMemPool, OutputSize, SubgroupSize>;
    typealias Act = IdentityActivation<float>;
    typealias LinearLayer = FFLayer<float, V2, V1, Storage, Act, true>;

    let storage = Storage(params);
    let layer = LinearLayer(0, 2);  // weights at 0, bias at 2

    float featsArr[InputSize] = { x0, x1 };
    let feats = V2(featsArr);

    let predV = layer.eval<Storage>(storage, feats);

    // Only first thread writes result
    if (gtid == 0)
    {
        result[0] = predV[0];
    }
}

// Compute shader entry point for evaluating loss
[shader("compute")]
[numthreads(32, 1, 1)]
void compute_eval_loss(
    uint3 tid : SV_DispatchThreadID,
    uint gtid : SV_GroupIndex,
    RWStructuredBuffer<float> params,
    StructuredBuffer<float> xs,
    StructuredBuffer<float> ys,
    RWStructuredBuffer<float> loss_out,
    uniform int count)
{
    typealias ShMemPool = SharedMemoryPool<ShMemSizeLayer>;
    typealias V2 = WaveTangledVector<float, ShMemPool, InputSize, SubgroupSize>;
    typealias V1 = WaveTangledVector<float, ShMemPool, OutputSize, SubgroupSize>;
    typealias Act = IdentityActivation<float>;
    typealias LinearLayer = FFLayer<float, V2, V1, Storage, Act, true>;

    let storage = Storage(params);
    let layer = LinearLayer(0, 2);

    float sum = 0.0;
    for (int i = 0; i < count; i++)
    {
        let x = xs[i];
        float featsArr[InputSize] = { x, x * x };
        let feats = V2(featsArr);

        let predV = layer.eval<Storage>(storage, feats);
        let pred = predV[0];
        let target = ys[i];

        let err = pred - target;
        sum += err * err;
    }

    if (gtid == 0)
    {
        loss_out[0] = sum / float(count);
    }
}

// Compute shader entry point for training step
[shader("compute")]
[numthreads(32, 1, 1)]
void compute_train_step(
    uint3 tid : SV_DispatchThreadID,
    uint gtid : SV_GroupIndex,
    RWStructuredBuffer<float> params,
    RWStructuredBuffer<float> grads,
    StructuredBuffer<float> xs,
    StructuredBuffer<float> ys,
    RWStructuredBuffer<float> loss_out,
    uniform int count,
    uniform float learningRate)
{
    typealias ShMemPool = SharedMemoryPool<ShMemSizeLayer>;
    typealias V2 = WaveTangledVector<float, ShMemPool, InputSize, SubgroupSize>;
    typealias V1 = WaveTangledVector<float, ShMemPool, OutputSize, SubgroupSize>;
    typealias Act = IdentityActivation<float>;
    typealias LinearLayer = FFLayer<float, V2, V1, Storage, Act, true>;

    let storage = Storage(params);
    let layer = LinearLayer(0, 2);

    // Clear grads (only first thread)
    if (gtid == 0)
    {
        for (int i = 0; i < PARAM_COUNT; i++)
            grads[i] = 0.0;
    }
    GroupMemoryBarrierWithGroupSync();

    // Accumulate analytic grads for y = w0*x + w1*x^2 + b
    float g0 = 0.0;
    float g1 = 0.0;
    float gb = 0.0;
    float lossSum = 0.0;

    for (int i = 0; i < count; i++)
    {
        let x = xs[i];
        let t = ys[i];

        float featsArr[InputSize] = { x, x * x };
        let feats = V2(featsArr);

        let predV = layer.eval<Storage>(storage, feats);
        let pred = predV[0];

        let err = pred - t;
        lossSum += err * err;

        g0 += 2.0 * err * x;
        g1 += 2.0 * err * (x * x);
        gb += 2.0 * err;
    }

    // Only first thread writes and updates
    if (gtid == 0)
    {
        let invN = 1.0 / float(count);
        grads[0] = g0 * invN;
        grads[1] = g1 * invN;
        grads[2] = gb * invN;

        // SGD update
        for (int i = 0; i < PARAM_COUNT; i++)
        {
            params[i] = params[i] - learningRate * grads[i];
        }

        loss_out[0] = lossSum * invN;
    }
}

// Multi-workgroup compute shader for testing atomicAdd correctness.
// Each workgroup processes samples and uses atomicAdd to accumulate gradients.
// Dispatch with (sample_count / 32, 1, 1) workgroups.
[shader("compute")]
[numthreads(32, 1, 1)]
void compute_grad_multiworkgroup(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    RWStructuredBuffer<float> params,
    RWStructuredBuffer<float> grads,
    StructuredBuffer<float> xs,
    StructuredBuffer<float> ys,
    uniform int count)
{
    typealias ShMemPool = SharedMemoryPool<ShMemSizeLayer>;
    typealias V2 = WaveTangledVector<float, ShMemPool, InputSize, SubgroupSize>;
    typealias V1 = WaveTangledVector<float, ShMemPool, OutputSize, SubgroupSize>;
    typealias Act = IdentityActivation<float>;
    typealias LinearLayer = FFLayer<float, V2, V1, Storage, Act, true>;

    // Each workgroup processes one sample (using gtid=0 for the computation)
    let idx = int(gid.x);
    if (idx >= count)
        return;

    let storage = Storage(params);
    let layer = LinearLayer(0, 2);

    let x = xs[idx];
    let t = ys[idx];

    float featsArr[InputSize] = { x, x * x };
    let feats = V2(featsArr);

    let predV = layer.eval<Storage>(storage, feats);
    let pred = predV[0];

    let err = pred - t;

    // Only first thread in workgroup does the atomicAdd
    if (gtid == 0)
    {
        let scale = 2.0 / float(count);
        let g0 = scale * err * x;
        let g1 = scale * err * (x * x);
        let gb = scale * err;

        InterlockedAdd(grads[0], g0);
        InterlockedAdd(grads[1], g1);
        InterlockedAdd(grads[2], gb);
    }
}

// Clear gradients buffer
[shader("compute")]
[numthreads(1, 1, 1)]
void compute_clear_grads(RWStructuredBuffer<float> grads)
{
    for (int i = 0; i < PARAM_COUNT; i++)
        grads[i] = 0.0;
}
