// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// SlangPy test for FFLayer autodiff backward pass (Option 2 design).
// Verifies that autodiff correctly computes gradients through eval<S>().
//
// We fit a quadratic polynomial y = 2*x^2 - 0.5*x + 0.25 using a single linear layer over
// features [x, x^2], and verify training converges.

import slangpy;
import neural;

typealias Storage = StructuredBufferStorage<float>;
typealias V2 = InlineVector<float, 2>;
typealias V1 = InlineVector<float, 1>;
typealias Act = IdentityActivation<float>;

// Linear layer: Input=2 (x, x^2), Output=1 (y), with bias
// Parameters: weights (1x2) + bias (1) = 3 params
typealias LinearLayer = FFLayer<float, V2, V1, Storage, LinearLayout, Act, true>;

static const int PARAM_COUNT = LinearLayer.ParameterCount;

int get_param_count()
{
    return PARAM_COUNT;
}

float eval_loss(
    RWStructuredBuffer<float> params,
    StructuredBuffer<float> xs,
    StructuredBuffer<float> ys,
    int count)
{
    let storage = Storage(params);
    // Option 2: only addresses in constructor
    // weights at 0 (2 floats), bias at 2 (1 float)
    let layer = LinearLayer(0, 2);

    float sum = 0.0;
    [MaxIters(1024)]
    for (int i = 0; i < count; i++)
    {
        let x = xs[i];

        float featsArr[2] = { x, x * x };
        let feats = V2(featsArr);

        // Storage passed to eval<S>()
        let predV = layer.eval<Storage>(storage, feats);
        let pred = predV[0];
        let target = ys[i];

        let err = pred - target;
        sum += err * err;
    }

    return sum / float(count);
}

float train_step(
    RWStructuredBuffer<float> params,
    RWStructuredBuffer<float> grads,
    no_diff StructuredBuffer<float> xs,
    no_diff StructuredBuffer<float> ys,
    no_diff int count,
    no_diff float learningRate)
{
    let pStorage = Storage(params);
    let gStorage = Storage(grads);

    // Clear gradient buffer
    [MaxIters(1024)]
    for (int i = 0; i < PARAM_COUNT; i++)
        grads[i] = 0.0;

    // Option 2: only addresses in constructor
    let layer = LinearLayer(0, 2);

    // Accumulate analytic grads for y = w0*x + w1*x^2 + b, loss = mean((y - t)^2)
    float g0 = 0.0;
    float g1 = 0.0;
    float gb = 0.0;

    float lossSum = 0.0;

    [MaxIters(1024)]
    for (int i = 0; i < count; i++)
    {
        let x = xs[i];
        let t = ys[i];

        float featsArr[2] = { x, x * x };
        let feats = V2(featsArr);

        // Storage passed to eval<S>()
        let predV = layer.eval<Storage>(pStorage, feats);
        let pred = predV[0];

        let err = pred - t;
        lossSum += err * err;

        g0 += 2.0 * err * x;
        g1 += 2.0 * err * (x * x);
        gb += 2.0 * err;
    }

    let invN = 1.0 / float(count);
    grads[0] = g0 * invN;
    grads[1] = g1 * invN;
    grads[2] = gb * invN;

    // Simple SGD update: params -= lr * grads
    [MaxIters(1024)]
    for (int i = 0; i < PARAM_COUNT; i++)
    {
        params[i] = params[i] - learningRate * grads[i];
    }

    return lossSum * invN;
}

// Multi-workgroup compute shader for testing atomicAdd correctness.
// Each thread processes one sample and uses atomicAdd to accumulate gradients.
// Dispatch with (sample_count / 32, 1, 1) workgroups.
[shader("compute")]
[numthreads(32, 1, 1)]
void compute_grad_multiworkgroup(
    uint3 tid : SV_DispatchThreadID,
    RWStructuredBuffer<float> params,
    RWStructuredBuffer<float> grads,
    StructuredBuffer<float> xs,
    StructuredBuffer<float> ys,
    uniform int count)
{
    let idx = int(tid.x);
    if (idx >= count)
        return;

    let storage = Storage(params);
    let layer = LinearLayer(0, 2);

    let x = xs[idx];
    let t = ys[idx];

    float featsArr[2] = { x, x * x };
    let feats = V2(featsArr);

    let predV = layer.eval<Storage>(storage, feats);
    let pred = predV[0];

    let err = pred - t;

    // Compute per-sample gradients (scaled by 2/count for MSE)
    let scale = 2.0 / float(count);
    let g0 = scale * err * x;
    let g1 = scale * err * (x * x);
    let gb = scale * err;

    // atomicAdd to accumulate gradients from all threads
    InterlockedAdd(grads[0], g0);
    InterlockedAdd(grads[1], g1);
    InterlockedAdd(grads[2], gb);
}
