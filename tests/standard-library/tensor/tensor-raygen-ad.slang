// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// TEST:INTERPRET(hgvm): -api vulkan

import slang.tensor.differentiable;
import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;
import slang.pipeline_rt;

/// Exercises tensor state across both an early return and a reverse-mode loop.
[Differentiable]
float3 tensorPolynomial(uint2 pixel, Tensor<float3, 1> values)
{
    if (pixel.y == 2)
        return float3(0.0f);
    uint material = pixel.x % 6;
    float3 result = values[material];
    [MaxIters(2)]
    for (uint power = 0; power < 1 + pixel.x / 6; ++power)
        result *= values[material];
    return result;
}

[host]
void main()
{
    let values = Tensor<float3, 1>(6u);
    let gradient = values.allocateDifferential();
    let direction = values.allocateDifferential();
    gradient.fill(float3(1.0f));
    direction.fill(float3(1.0f, -2.0f, 0.5f));
    Grid(6u).dispatch(
        (uint index) =>
        { values[index] = float3(1.0f, 2.0f, 3.0f) + float(index * 3); });
    let primal = Tensor<float3, 2>(uint2(12u, 3u));
    let tangent = Tensor<float3, 2>(primal.shape);
    let forward = DifferentialPtrPair<Tensor<float3, 1>>(values, direction);
    let backward = DifferentialPtrPair<Tensor<float3, 1>>(values, gradient);
    // Capture ordinary tensors and differential pairs directly, with no handle conversions.
    Grid(primal.shape)
        .dispatchRays(
            (uint2 pixel) =>
            {
                primal[pixel] = tensorPolynomial(pixel, values);
                tangent[pixel] = fwd_diff(tensorPolynomial)(pixel, forward).d;
                bwd_diff(tensorPolynomial)(pixel, backward, float3(0.25f, 0.5f, 1.0f));
            });
    let errors = Tensor<uint, 2>(primal.shape);
    Grid(primal.shape)
        .dispatch(
            (uint2 pixel) =>
            {
                float3 value = values[pixel.x % 6];
                float3 expected = pixel.x < 6 ? value * value : value * value * value;
                float3 derivative = pixel.x < 6 ? 2.0f * value : 3.0f * value * value;
                if (pixel.y == 2)
                {
                    expected = float3(0.0f);
                    derivative = float3(0.0f);
                }
                errors[pixel] =
                    any(primal[pixel] != expected) ||
                            any(tangent[pixel] != derivative * float3(1.0f, -2.0f, 0.5f))
                        ? 1u
                        : 0u;
            });
    for (uint index = 0; index < errors.elementCount(); ++index)
        assert(errors.readBack(index) == 0);
    for (uint index = 0; index < 6; ++index)
    {
        float3 value = values.readBack(index);
        // Two active rows contribute one square and one cube per parameter, adding to 1.
        float3 expected =
            1.0f + 2.0f * (2.0f * value + 3.0f * value * value) * float3(0.25f, 0.5f, 1.0f);
        assert(!any(gradient.readBack(index) != expected));
    }
}
