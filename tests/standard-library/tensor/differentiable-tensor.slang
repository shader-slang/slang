// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Known failure: SquareTensorElement<Rank> fails callable differentiability conformance.
// Re-enable these cases once its synthesized callable satisfies IDifferentiableFuncWithDiffThis.
// DISABLE_TEST:INTERPRET(hgvm): -api vulkan
// DISABLE_TEST:INTERPRET(hgvm): -api d3d12
// DISABLE_TEST:INTERPRET(hgvm): -api cuda

import slang.tensor.differentiable;
import slang.runtime.core.simple_assert;

struct SquareTensorElement<let Rank : int>
    : IDifferentiableFuncWithDiffThis<float, vector<uint, Rank>, float>
{
    typealias Differential = SquareTensorElement<Rank>;

    [Differentiable]
    float operator()(vector<uint, Rank> index, float value) { return value * value; }
}

[host]
void checkShape<let Rank : int>(vector<uint, Rank> shape)
{
    let input = Tensor<float, Rank>(shape);
    let output = Tensor<float, Rank>(shape);
    let inputGradient = input.allocateDifferential();
    let outputGradient = output.allocateDifferential();
    input.fill(3.0f);
    inputGradient.fill(7.0f);
    outputGradient.fill(2.0f);
    let function = DifferentialPtrPair<SquareTensorElement<Rank>>(
        SquareTensorElement<Rank>(),
        SquareTensorElement<Rank>());
    let copied = input.copy();
    let mapped = input.dmap(function.p);
    assert(!any(copied.shape != shape) && !any(mapped.shape != shape));
    copied.fill(5.0f);
    if (input.elementCount() != 0)
    {
        uint last = input.elementCount() - 1;
        assert(readBack(input.buffer, last) == 3.0f);
        assert(readBack(copied.buffer, last) == 5.0f);
        assert(readBack(mapped.buffer, last) == 9.0f);
    }
    let backward = __apply(input.dmapTo)(
        DifferentialPtrPair<Tensor<float, Rank>>(input, inputGradient),
        DifferentialPtrPair<Tensor<float, Rank>>(output, outputGradient),
        function);
    __none_t dInput;
    __none_t dOutput;
    __none_t dFunction;
    backward(dInput, dOutput, dFunction);
    if (input.elementCount() != 0)
    {
        uint last = input.elementCount() - 1;
        assert(readBack(output.buffer, last) == 9.0f);
        assert(readBack(inputGradient.buffer, last) == 19.0f);
    }

    let sum = Tensor<float, 1>(1u);
    let sumGradient = sum.allocateDifferential();
    sum.fill(-1.0f);
    sumGradient.fill(3.0f);
    let sumBackward = __apply(input.dsumTo)(
        DifferentialPtrPair<Tensor<float, Rank>>(input, inputGradient),
        DifferentialPtrPair<Tensor<float, 1>>(sum, sumGradient));
    sumBackward(dInput, dOutput);
    assert(readBack(sum.buffer, 0) == 3.0f * float(input.elementCount()));
    if (input.elementCount() != 0)
        assert(readBack(inputGradient.buffer, input.elementCount() - 1) == 22.0f);

    let allocatedSum = input.dsum();
    assert(allocatedSum.elementCount() == 1 && allocatedSum.shape[0] == 1u);
    assert(readBack(allocatedSum.buffer, 0) == 3.0f * float(input.elementCount()));
    let appliedSum =
        __apply(input.dsum)(DifferentialPtrPair<Tensor<float, Rank>>(input, inputGradient));
    let pairedSum = appliedSum._0;
    assert(pairedSum.p.elementCount() == 1 && pairedSum.d.elementCount() == 1);
    assert(readBack(pairedSum.p.buffer, 0) == 3.0f * float(input.elementCount()));
    assert(readBack(pairedSum.d.buffer, 0) == 0.0f);
    pairedSum.d.fill(5.0f);
    appliedSum._1(dInput);
    if (input.elementCount() != 0)
        assert(readBack(inputGradient.buffer, input.elementCount() - 1) == 27.0f);
}

[host]
void checkOutstandingSnapshots()
{
    var input = Tensor<float, 1>(65u);
    input.reserve(100u);
    var inputGradient = input.allocateDifferential();
    let originalGradient = inputGradient.buffer;
    let firstOutput = Tensor<float, 1>(65u);
    let secondOutput = Tensor<float, 1>(65u);
    let firstSeed = firstOutput.allocateDifferential();
    let secondSeed = secondOutput.allocateDifferential();
    input.fill(3.0f);
    inputGradient.fill(7.0f);
    firstSeed.fill(1.0f);
    secondSeed.fill(2.0f);
    let function = DifferentialPtrPair<SquareTensorElement<1>>(
        SquareTensorElement<1>(),
        SquareTensorElement<1>());
    let first = __apply(input.dmapTo)(
        DifferentialPtrPair<Tensor<float, 1>>(input, inputGradient),
        DifferentialPtrPair<Tensor<float, 1>>(firstOutput, firstSeed),
        function);

    // This copy aliases primal memory. A shallow context copy would incorrectly observe 10.
    let alias = input;
    alias.fill(10.0f);
    let second = __apply(input.dmapTo)(
        DifferentialPtrPair<Tensor<float, 1>>(input, inputGradient),
        DifferentialPtrPair<Tensor<float, 1>>(secondOutput, secondSeed),
        function);
    input.resize(32u); // Same allocation, changed logical shape.
    input.fill(100.0f);
    input.resize(200u); // Replace both variables' allocations after saving the contexts.
    inputGradient.resize(200u);
    input.fill(-1.0f);
    inputGradient.fill(-1.0f);

    __none_t dInput;
    __none_t dOutput;
    __none_t dFunction;
    first(dInput, dOutput, dFunction);
    second(dInput, dOutput, dFunction);
    // No readback separated the two forward passes, mutations, and backward dispatches.
    // Contributions add to the original destination: 7 + 2*3*1 + 2*10*2 = 53.
    assert(readBack(originalGradient, 0) == 53.0f);
    assert(readBack(originalGradient, 64) == 53.0f);
    assert(readBack(inputGradient.buffer, 0) == -1.0f);
    assert(readBack(firstOutput.buffer, 64) == 9.0f);
    assert(readBack(secondOutput.buffer, 64) == 100.0f);
}

[host]
void checkFiniteDifference()
{
    let input = Tensor<float, 1>(1u);
    let output = Tensor<float, 1>(1u);
    let gradient = input.allocateDifferential();
    let seed = output.allocateDifferential();
    input.fill(1.25f);
    seed.fill(1.0f);
    let function = SquareTensorElement<1>();
    let backward = __apply(input.dmapTo)(
        DifferentialPtrPair<Tensor<float, 1>>(input, gradient),
        DifferentialPtrPair<Tensor<float, 1>>(output, seed),
        DifferentialPtrPair<SquareTensorElement<1>>(function, function));
    __none_t dInput;
    __none_t dOutput;
    __none_t dFunction;
    backward(dInput, dOutput, dFunction);
    float derivative = readBack(gradient.buffer, 0);
    input.fill(1.251f);
    input.dmapTo(output, function);
    float plus = readBack(output.buffer, 0);
    input.fill(1.249f);
    input.dmapTo(output, function);
    float minus = readBack(output.buffer, 0);
    float error = derivative - (plus - minus) / 0.002f;
    assert(error > -0.001f && error < 0.001f);
}

[host]
void checkAllocatingMap()
{
    let input = Tensor<float, 1>(65u);
    let gradient = input.allocateDifferential();
    input.fill(3.0f);
    gradient.fill(7.0f);
    let function = SquareTensorElement<1>();
    let applied = __apply(input.dmap)(
        DifferentialPtrPair<Tensor<float, 1>>(input, gradient),
        DifferentialPtrPair<SquareTensorElement<1>>(function, function));
    let output = applied._0;
    assert(readBack(output.p.buffer, 64) == 9.0f);
    assert(readBack(output.d.buffer, 64) == 0.0f);
    output.d.fill(2.0f);
    input.fill(100.0f);
    __none_t dInput;
    __none_t dFunction;
    applied._1(dInput, dFunction);
    assert(readBack(gradient.buffer, 64) == 19.0f);
}

[host]
[BackwardDifferentiable]
void allocatingMapObjective(Tensor<float, 1> input, Tensor<float, 1> loss)
{
    let squared = input.dmap((uint1 index, float value) ~ => value * value);
    let fourthPower = squared.dmap((uint1 index, float value) ~ => value * value);
    let summed = fourthPower.dsum();
    summed.dsumTo(loss);
}

[host]
void checkAllocatingMapComposition()
{
    let input = Tensor<float, 1>(65u);
    let gradient = input.allocateDifferential();
    let loss = Tensor<float, 1>(1u);
    let seed = loss.allocateDifferential();
    input.fill(3.0f);
    gradient.fill(7.0f);
    seed.fill(2.0f);
    bwd_diff(allocatingMapObjective)(
        DifferentialPtrPair<Tensor<float, 1>>(input, gradient),
        DifferentialPtrPair<Tensor<float, 1>>(loss, seed));
    assert(readBack(loss.buffer, 0) == 81.0f * 65.0f);
    assert(readBack(gradient.buffer, 64) == 223.0f);
}

[host]
void main()
{
    checkShape<1>(0u);
    checkShape<1>(1u);
    checkShape<1>(65u);
    checkShape<1>(129u);
    checkShape<2>(uint2(5, 13));
    checkShape<3>(uint3(5, 3, 2));
    checkShape<4>(uint4(3, 2, 2, 2));
    checkOutstandingSnapshots();
    checkFiniteDifference();
    checkAllocatingMap();
    checkAllocatingMapComposition();
}
