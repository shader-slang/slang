// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// SlangPy test for FFLayer autodiff backward pass.
// Uses PointerAddress (float*) for CUDA compatibility.

import slangpy;
import slang.neural;

typealias Address = PointerAddress<float>;
typealias V2 = InlineVector<float, 2>;
typealias V1 = InlineVector<float, 1>;
typealias Act = IdentityActivation<float>;

typealias LinearLayer = FFLayer<float, V2, V1, LinearLayout, Act>;

static const int PARAM_COUNT = LinearLayer.ParameterCount;

int get_param_count()
{
    return PARAM_COUNT;
}

float eval_loss(
    float* params_ptr,
    StructuredBuffer<float> xs,
    StructuredBuffer<float> ys,
    int count)
{
    let paramAddr = Address(params_ptr);
    let layer = LinearLayer();

    float sum = 0.0;
    [MaxIters(1024)]
    for (int i = 0; i < count; i++)
    {
        let x = xs[i];
        float featsArr[2] = { x, x * x };
        let feats = V2(featsArr);
        let predV = layer.eval<Address>(feats, paramAddr);
        let pred = predV[0];
        let target = ys[i];
        let err = pred - target;
        sum += err * err;
    }

    return sum / float(count);
}

float train_step(
    float* params_ptr,
    float* grads_ptr,
    no_diff StructuredBuffer<float> xs,
    no_diff StructuredBuffer<float> ys,
    no_diff int count,
    no_diff float learningRate)
{
    let pAddr = Address(params_ptr);
    let gAddr = Address(grads_ptr);

    [MaxIters(1024)]
    for (int i = 0; i < PARAM_COUNT; i++)
        grads_ptr[i] = 0.0;

    let layer = LinearLayer();

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
        let predV = layer.eval<Address>(feats, pAddr);
        let pred = predV[0];
        let err = pred - t;
        lossSum += err * err;
        g0 += 2.0 * err * x;
        g1 += 2.0 * err * (x * x);
        gb += 2.0 * err;
    }

    let invN = 1.0 / float(count);
    grads_ptr[0] = g0 * invN;
    grads_ptr[1] = g1 * invN;
    grads_ptr[2] = gb * invN;

    [MaxIters(1024)]
    for (int i = 0; i < PARAM_COUNT; i++)
        params_ptr[i] = params_ptr[i] - learningRate * grads_ptr[i];

    return lossSum * invN;
}

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_grad_multiworkgroup(
    uint3 tid : SV_DispatchThreadID,
    uniform float* params_ptr,
    RWStructuredBuffer<float> grads,
    StructuredBuffer<float> xs,
    StructuredBuffer<float> ys,
    uniform int count)
{
    let idx = int(tid.x);
    if (idx >= count)
        return;

    let paramAddr = Address(params_ptr);
    let layer = LinearLayer();

    let x = xs[idx];
    let t = ys[idx];
    float featsArr[2] = { x, x * x };
    let feats = V2(featsArr);
    let predV = layer.eval<Address>(feats, paramAddr);
    let pred = predV[0];
    let err = pred - t;

    let scale = 2.0 / float(count);
    InterlockedAdd(grads[0], scale * err * x);
    InterlockedAdd(grads[1], scale * err * (x * x));
    InterlockedAdd(grads[2], scale * err);
}
