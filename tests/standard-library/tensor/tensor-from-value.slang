// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12
// TEST:INTERPRET(hgvm): -api cuda

import slang.tensor.differentiable;
import slang.runtime.core.simple_assert;

[host]
[Differentiable]
Tensor<float3, 2> scaledSingleton(float3 value)
{
    return Tensor<float3, 2>.fromValue(2.0f * value);
}

[host]
[BackwardDifferentiable]
float scalarLoss(float value)
{
    let first = Tensor<float, 1>.fromValue(value);
    let second = Tensor<float, 1>.fromValue(3.0f * value);
    let squared = first.dmap((uint1 i, float x) ~ => x * x);
    // Two reads must accumulate into one singleton; both allocations contribute to value.
    return squared.dsum().readBack(0) + first.readBack(0) + second.readBack(0);
}

[host]
[BackwardDifferentiable]
float vectorLoss(float3 value)
{
    let tensor = scaledSingleton(value);
    return tensor.dmap((uint2 i, float3 v) ~ => v.x * v.x + 3.0f * v.y + v.z).dsum().readBack(0);
}

[host]
void main()
{
    // A value of zero still creates one initialized element, including for integer tensors.
    let integer = Tensor<uint, 1>.fromValue(0u);
    assert(integer.elementCount() == 1 && integer.shape.x == 1);
    assert(integer.readBack(0) == 0u);
    let rankThree = Tensor<float, 3>.fromValue(-2.0f);
    assert(!any(rankThree.shape != uint3(1u)) && rankThree.readBack(0) == -2.0f);
    let rankFour = Tensor<float, 4>.fromValue(5.0f);
    assert(!any(rankFour.shape != uint4(1u)) && rankFour.readBack(0) == 5.0f);

    let forward =
        fwd_diff(scaledSingleton)(diffPair(float3(2.0f, 3.0f, 4.0f), float3(0.5f, -1.0f, 2.0f)));
    assert(!any(forward.p.shape != uint2(1u)));
    assert(!any(forward.p.readBack(0) != float3(4.0f, 6.0f, 8.0f)));
    assert(!any(forward.d.readBack(0) != float3(1.0f, -2.0f, 4.0f)));
    forward.d.fill(float3(0.0f));
    assert(!any(forward.p.readBack(0) != float3(4.0f, 6.0f, 8.0f)));

    var scalar = diffPair(2.0f, 0.0f);
    assert(scalarLoss(scalar.p) == 12.0f);
    bwd_diff(scalarLoss)(scalar, 2.0f);
    assert(scalar.p == 2.0f && scalar.d == 16.0f);
    // A later call gets fresh zeroed output adjoints.
    scalar = diffPair(-1.0f, 0.0f);
    bwd_diff(scalarLoss)(scalar, 3.0f);
    assert(scalar.p == -1.0f && scalar.d == 6.0f);

    var vector = diffPair(float3(2.0f, 3.0f, 4.0f), float3(0.0f));
    assert(vectorLoss(vector.p) == 42.0f);
    bwd_diff(vectorLoss)(vector, 0.5f);
    assert(!any(vector.d != float3(8.0f, 3.0f, 1.0f)));
}
