// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api cuda

import slang.tensor.differentiable;
import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;

[BackwardDifferentiable]
float tensorVectorRead(Tensor<float2, 1> parameters)
{
    float2 value = parameters[0];
    return 2.0f * value.x + 3.0f * value.y;
}

[host]
void checkSharedVectorReads()
{
    let parameters = Tensor<float2, 1>(1u);
    let gradient = parameters.allocateDifferential();
    parameters.fill(float2(4.0f, 5.0f));
    gradient.fill(float2(1.0f));
    Grid(65u).dispatch(
        (uint index) =>
        {
            bwd_diff(tensorVectorRead)(
                DifferentialPtrPair<Tensor<float2, 1>>(parameters, gradient),
                1.0f);
        });
    float2 result = readBack(gradient.buffer, 0);
    assert(!any(result != float2(131.0f, 196.0f)));
}

[host]
[BackwardDifferentiable]
void allocatingCapturedObjective(
    Tensor<float, 1> input,
    Tensor<float, 1> parameters,
    Tensor<float, 1> loss)
{
    let prediction = input.dmap((uint1 index, float value) ~ => parameters[0] * value * value);
    let summed = prediction.dsum();
    summed.dsumTo(loss);
}

[host]
void checkAllocatingCapturedMap()
{
    let input = Tensor<float, 1>(65u);
    let parameters = Tensor<float, 1>(1u);
    let loss = Tensor<float, 1>(1u);
    let inputGradient = input.allocateDifferential();
    let parameterGradient = parameters.allocateDifferential();
    let lossGradient = loss.allocateDifferential();
    input.fill(3.0f);
    parameters.fill(4.0f);
    inputGradient.fill(7.0f);
    parameterGradient.fill(5.0f);
    lossGradient.fill(2.0f);
    bwd_diff(allocatingCapturedObjective)(
        DifferentialPtrPair<Tensor<float, 1>>(input, inputGradient),
        DifferentialPtrPair<Tensor<float, 1>>(parameters, parameterGradient),
        DifferentialPtrPair<Tensor<float, 1>>(loss, lossGradient));
    assert(readBack(loss.buffer, 0) == 36.0f * 65.0f);
    assert(readBack(inputGradient.buffer, 64) == 55.0f);
    assert(readBack(parameterGradient.buffer, 0) == 1175.0f);
}

[host]
void main()
{
    checkSharedVectorReads();
    checkAllocatingCapturedMap();
}
