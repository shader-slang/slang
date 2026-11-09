// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12
// TEST:INTERPRET(hgvm): -api cuda

import slang.tensor.differentiable;
import slang.runtime.core.simple_assert;

[host]
[BackwardDifferentiable]
float squaredRead(Tensor<float, 2> values, uint index)
{
    float value = values.readBack(index);
    return value * value;
}

[host]
[BackwardDifferentiable]
float vectorRead(Tensor<float3, 2> values, uint index)
{
    float3 value = values.readBack(index);
    return value.x * value.x + 3.0f * value.y + value.z;
}

[host]
[BackwardDifferentiable]
float squaredSubscript(Tensor<float, 2> values, uint2 index)
{
    float value = values[index];
    return value * value;
}

[host]
[BackwardDifferentiable]
float vectorSubscript(Tensor<float3, 2> values, uint2 index)
{
    float3 value = values[index];
    return value.x * value.x + 3.0f * value.y + value.z;
}

[host]
void main()
{
    let values = Tensor<float, 2>(uint2(3u, 2u));
    values.fill(3.0f);
    let gradient = values.allocateDifferential();
    gradient.fill(7.0f);
    let pair = DifferentialPtrPair<Tensor<float, 2>>(values, gradient);
    assert(squaredRead(values, 4u) == 9.0f);

    // A flat index in the second row must preserve every other element and existing gradients.
    bwd_diff(squaredRead)(pair, 4u, 2.0f);
    bwd_diff(squaredRead)(pair, 4u, 0.5f);
    for (uint index = 0; index < values.elementCount(); ++index)
    {
        assert(values.readBack(index) == 3.0f);
        assert(gradient.readBack(index) == (index == 4u ? 22.0f : 7.0f));
    }

    let vectors = Tensor<float3, 2>(uint2(3u, 2u));
    vectors.fill(float3(2.0f, 3.0f, 4.0f));
    let vectorGradient = vectors.allocateDifferential();
    vectorGradient.fill(float3(1.0f));
    let vectorPair = DifferentialPtrPair<Tensor<float3, 2>>(vectors, vectorGradient);
    assert(vectorRead(vectors, 5u) == 17.0f);
    bwd_diff(vectorRead)(vectorPair, 5u, 2.0f);
    for (uint index = 0; index < vectors.elementCount(); ++index)
    {
        float3 expected = index == 5u ? float3(9.0f, 7.0f, 3.0f) : float3(1.0f);
        assert(!any(vectorGradient.readBack(index) != expected));
    }

    // Ordinary readback also works for element types that have no differential.
    let integers = Tensor<uint, 1>(3u);
    integers.fill(42u);
    assert(integers.readBack(2u) == 42u);

    // Coordinate indexing transfers just the selected element, including host writes and AD.
    values[uint2(1u, 1u)] = 5.0f;
    values[uint2(1u, 1u)] += 1.0f;
    assert(values[uint2(1u, 1u)] == 6.0f && values.readBack(3u) == 3.0f);
    gradient.fill(0.0f);
    bwd_diff(squaredSubscript)(pair, uint2(1u, 1u), 2.0f);
    assert(gradient[uint2(1u, 1u)] == 24.0f && gradient[uint2(0u, 1u)] == 0.0f);

    vectors[uint2(2u, 1u)] = float3(2.0f, 8.0f, 4.0f);
    assert(!any(vectors[uint2(2u, 1u)] != float3(2.0f, 8.0f, 4.0f)));
    vectorGradient.fill(float3(0.0f));
    bwd_diff(vectorSubscript)(vectorPair, uint2(2u, 1u), 2.0f);
    assert(!any(vectorGradient[uint2(2u, 1u)] != float3(8.0f, 6.0f, 2.0f)));
    assert(!any(vectorGradient[uint2(1u, 1u)] != float3(0.0f)));
}
