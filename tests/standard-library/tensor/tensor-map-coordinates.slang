// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12
// TEST:INTERPRET(hgvm): -api cuda

import slang.tensor.differentiable;
import slang.runtime.core.simple_assert;

[host]
[BackwardDifferentiable]
float coordinateObjective(Tensor<float, 2> input)
{
    let mapped = input.dmap(
        (uint2 pixel, float value) ~ => value * value * float(1u + pixel.x + 10u * pixel.y));
    return mapped.dsum().readBack(0);
}

[host]
void main()
{
    let input = Tensor<float, 2>(uint2(3u, 2u));
    input.fill(2.0f);
    let gradient = input.allocateDifferential();
    gradient.fill(7.0f);
    assert(coordinateObjective(input) == 168.0f);
    bwd_diff(coordinateObjective)(DifferentialPtrPair<Tensor<float, 2>>(input, gradient), 3.0f);
    for (uint y = 0; y < 2; ++y)
        for (uint x = 0; x < 3; ++x)
            assert(gradient.readBack(x + 3u * y) == 7.0f + 12.0f * float(1u + x + 10u * y));
}
