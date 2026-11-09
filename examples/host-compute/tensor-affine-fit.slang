// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.tensor.differentiable;
import slang.compute.buffer_utils;
import slang.runtime;

public static const uint kTensorAffineSampleCount = 129;

public float tensorInputSample(uint index)
{
    return float(index) / 64.0f - 1.0f;
}

[host]
[BackwardDifferentiable]
public float computeLoss(
    Tensor<float, 1> samples,
    no_diff Tensor<float, 1> targets,
    Tensor<float, 1> parameters)
{
    let prediction = samples.dmap(
        (uint1 index, float value) ~ =>
        { return parameters[0] * value + parameters[1]; });

    var squaredError = prediction.dmap(
        (uint1 index, float value) ~ =>
        {
            float target = no_diff targets[index.x];
            float residual = value - target;
            return residual * residual;
        });

    let scalarLoss = squaredError.dsum();
    return scalarLoss.readBack(0);
}

public struct TensorAffineFitResult
{
    public float initialLoss;
    public float finalLoss;
    public float firstGainGradient;
    public float firstBiasGradient;
    public float gain;
    public float bias;
}

/// Runs a deterministic SGD loop. Parameter updates happen only after backward completes.
[host]
public TensorAffineFitResult fitTensorAffine(uint iterations = 80)
{
    assert(iterations > 0);
    let samples = Tensor<float, 1>(kTensorAffineSampleCount);
    let parameters = Tensor<float, 1>(2u);

    let targets = Tensor<float, 1>(kTensorAffineSampleCount);

    let sampleBuffer = samples.buffer;
    Grid(kTensorAffineSampleCount)
        .dispatch(
            (uint index) =>
            {
                sampleBuffer[index] = tensorInputSample(index);
                targets.buffer[index] = 2.0f * sampleBuffer[index] + 1.0f;
            });
    parameters.fill(0.0f);

    let dSamples = samples.allocateDifferential();
    let dParameters = parameters.allocateDifferential();

    TensorAffineFitResult result = {};
    for (uint iteration = 0; iteration < iterations; ++iteration)
    {
        dSamples.fill(0.0f);
        dParameters.fill(0.0f);
        let samplePair = DifferentialPtrPair<Tensor<float, 1>>(samples, dSamples);
        let parameterPair = DifferentialPtrPair<Tensor<float, 1>>(parameters, dParameters);

        bwd_diff(computeLoss)(samplePair, targets, parameterPair, 1.f);

        if (iteration == 0)
        {
            result.initialLoss = computeLoss(samples, targets, parameters);
            result.firstGainGradient = dParameters.readBack(0);
            result.firstBiasGradient = dParameters.readBack(1);
        }
        let gradient = dParameters.buffer;
        parameters.buffer.mapInPlaceIndexed(
            2,
            (uint index, float value) =>
            { return value - 0.002f * gradient[index]; });
    }
    let loss = computeLoss(samples, targets, parameters);
    result.finalLoss = loss;
    result.gain = parameters.readBack(0);
    result.bias = parameters.readBack(1);
    return result;
}

[host]
void main()
{
    let result = fitTensorAffine();
    print(format("tensor affine fit: loss {} -> {}", result.initialLoss, result.finalLoss));
    print(format("gain {}, bias {} (target: 2, 1)", result.gain, result.bias));
    assert(result.finalLoss >= 0.0f && result.finalLoss < 1e-8f);
}
