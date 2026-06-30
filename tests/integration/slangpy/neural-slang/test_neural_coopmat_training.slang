// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// SlangPy test for FFLayer with CoopMat (WaveTangledVector) backend.

import slang.neural;

static const int InputSize = 2;
static const int OutputSize = 1;
static const int SubgroupSize = 32;
static const int BatchSize = 32;

typealias ShMemSize = SharedMemorySize<float, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, BatchSize / SubgroupSize>;
typealias ShMemSizeLayer = ShMemSize.OfLayer1<InputSize, OutputSize>;

static const int PARAM_COUNT = 3;

int get_param_count()
{
    return PARAM_COUNT;
}

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_forward_pass(
    uint3 tid : SV_DispatchThreadID,
    uint gtid : SV_GroupIndex,
    uniform float* params,
    RWStructuredBuffer<float> result,
    uniform float x0,
    uniform float x1)
{
    // CoopMat types must be declared per entry point — WaveTangledVector
    // shared memory layout is tied to the compute shader's numthreads.
    typealias V2 = WaveTangledVector<float, ShMemSizeLayer, InputSize, SubgroupSize>;
    typealias V1 = WaveTangledVector<float, ShMemSizeLayer, OutputSize, SubgroupSize>;
    typealias Act = IdentityActivation<float>;
    typealias LinearLayer = FFLayer<float, V2, V1, LinearLayout, Act>;
    typealias Address = PointerAddress<float>;

    let paramAddr = Address(params);
    let layer = LinearLayer();

    float featsArr[InputSize] = { x0, x1 };
    let feats = V2(featsArr);

    let predV = layer.eval<Address>(feats, paramAddr);

    if (gtid == 0)
    {
        result[0] = predV[0];
    }
}

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_eval_loss(
    uint3 tid : SV_DispatchThreadID,
    uint gtid : SV_GroupIndex,
    uniform float* params,
    StructuredBuffer<float> xs,
    StructuredBuffer<float> ys,
    RWStructuredBuffer<float> loss_out,
    uniform int count)
{
    typealias V2 = WaveTangledVector<float, ShMemSizeLayer, InputSize, SubgroupSize>;
    typealias V1 = WaveTangledVector<float, ShMemSizeLayer, OutputSize, SubgroupSize>;
    typealias Act = IdentityActivation<float>;
    typealias LinearLayer = FFLayer<float, V2, V1, LinearLayout, Act>;
    typealias Address = PointerAddress<float>;

    let paramAddr = Address(params);
    let layer = LinearLayer();

    float sum = 0.0;
    for (int i = 0; i < count; i++)
    {
        let x = xs[i];
        float featsArr[InputSize] = { x, x * x };
        let feats = V2(featsArr);

        let predV = layer.eval<Address>(feats, paramAddr);
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

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_train_step(
    uint3 tid : SV_DispatchThreadID,
    uint gtid : SV_GroupIndex,
    uniform float* params_handle,
    RWStructuredBuffer<float> params_rw,
    RWStructuredBuffer<float> grads,
    StructuredBuffer<float> xs,
    StructuredBuffer<float> ys,
    RWStructuredBuffer<float> loss_out,
    uniform int count,
    uniform float learningRate)
{
    typealias V2 = WaveTangledVector<float, ShMemSizeLayer, InputSize, SubgroupSize>;
    typealias V1 = WaveTangledVector<float, ShMemSizeLayer, OutputSize, SubgroupSize>;
    typealias Act = IdentityActivation<float>;
    typealias LinearLayer = FFLayer<float, V2, V1, LinearLayout, Act>;
    typealias Address = PointerAddress<float>;

    let paramAddr = Address(params_handle);
    let layer = LinearLayer();

    if (gtid == 0)
    {
        for (int i = 0; i < PARAM_COUNT; i++)
            grads[i] = 0.0;
    }
    GroupMemoryBarrierWithGroupSync();

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

        let predV = layer.eval<Address>(feats, paramAddr);
        let pred = predV[0];

        let err = pred - t;
        lossSum += err * err;

        g0 += 2.0 * err * x;
        g1 += 2.0 * err * (x * x);
        gb += 2.0 * err;
    }

    if (gtid == 0)
    {
        let invN = 1.0 / float(count);
        grads[0] = g0 * invN;
        grads[1] = g1 * invN;
        grads[2] = gb * invN;

        for (int i = 0; i < PARAM_COUNT; i++)
        {
            params_rw[i] = params_rw[i] - learningRate * grads[i];
        }

        loss_out[0] = lossSum * invN;
    }
}

[shader("compute")]
[numthreads(32, 1, 1)]
void compute_grad_multiworkgroup(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform float* params,
    RWStructuredBuffer<float> grads,
    StructuredBuffer<float> xs,
    StructuredBuffer<float> ys,
    uniform int count)
{
    typealias V2 = WaveTangledVector<float, ShMemSizeLayer, InputSize, SubgroupSize>;
    typealias V1 = WaveTangledVector<float, ShMemSizeLayer, OutputSize, SubgroupSize>;
    typealias Act = IdentityActivation<float>;
    typealias LinearLayer = FFLayer<float, V2, V1, LinearLayout, Act>;
    typealias Address = PointerAddress<float>;

    let idx = int(gid.x);
    if (idx >= count)
        return;

    let paramAddr = Address(params);
    let layer = LinearLayer();

    let x = xs[idx];
    let t = ys[idx];

    float featsArr[InputSize] = { x, x * x };
    let feats = V2(featsArr);

    let predV = layer.eval<Address>(feats, paramAddr);
    let pred = predV[0];

    let err = pred - t;

    if (gtid == 0)
    {
        let scale = 2.0 / float(count);
        let g0 = scale * err * x;
        let g1 = scale * err * (x * x);
        let gBias = scale * err;

        InterlockedAdd(grads[0], g0);
        InterlockedAdd(grads[1], g1);
        InterlockedAdd(grads[2], gBias);
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_clear_grads(RWStructuredBuffer<float> grads)
{
    for (int i = 0; i < PARAM_COUNT; i++)
        grads[i] = 0.0;
}
