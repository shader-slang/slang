// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12
// TEST:INTERPRET(hgvm): -api cuda

import slang.tensor.differentiable;
import slang.compute.buffer_utils;
import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;

/// Both map directions must preserve distinct input and output element differential types.
[host]
[BackwardDifferentiable]
float mappedLoss(Tensor<float, 2> input)
{
    let vectors =
        input.dmap((uint2 pixel, float value) ~ => float3(value, 2.0f * value, value * value));
    let scalars = vectors.dmap((uint2 pixel, float3 value) ~ => dot(value, value));
    return scalars.dsum().readBack(0);
}

[host]
void main()
{
    // A rectangular shape and more than one reduction block exercise coordinate preservation.
    let input = Tensor<float, 2>(uint2(17u, 9u));
    input.fill(2.0f);
    assert(mappedLoss(input) == 36.0f * 153.0f);
    let gradient = input.allocateDifferential();
    gradient.fill(0.25f);
    bwd_diff(mappedLoss)(DifferentialPtrPair<Tensor<float, 2>>(input, gradient), 0.5f);
    let errors = RWStructuredBuffer<float>::alloc(input.elementCount());
    Grid(input.shape)
        .dispatch(
            (uint2 pixel) =>
            {
                // d(5*x^2 + x^4)/dx is 52 at x=2; seed and existing adjoints must both apply.
                float error = gradient[pixel] - 26.25f;
                errors[input.flatIndex(pixel)] = error * error;
            });
    let total = RWStructuredBuffer<float>::alloc(1);
    errors.reduceTo(total, input.elementCount(), (float a, float b) => a + b);
    assert(total.readBack(0) < 1e-10f);
}
