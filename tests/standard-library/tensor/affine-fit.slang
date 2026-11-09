// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api cuda

import "../../../examples/host-compute/tensor-affine-fit.slang";
import slang.runtime.core.simple_assert;

[host]
void main()
{
    let result = fitTensorAffine();
    // computeLoss sums errors across all 129 samples. Their symmetry gives sum(x) = 0
    // and sum(x*x) = 2795/64. At gain = bias = 0, the loss is 4*sum(x*x) + 129,
    // the gain gradient is -4*sum(x*x), and the bias gradient is -2*129.
    assert(
        result.initialLoss > 4859.0f / 16.0f - 2e-5f &&
        result.initialLoss < 4859.0f / 16.0f + 2e-5f);
    assert(
        result.firstGainGradient > -2795.0f / 16.0f - 2e-5f &&
        result.firstGainGradient < -2795.0f / 16.0f + 2e-5f);
    assert(
        result.firstBiasGradient > -258.0f - 2e-5f && result.firstBiasGradient < -258.0f + 2e-5f);
    assert(result.finalLoss >= 0.0f && result.finalLoss < 1e-8f);
    assert(result.gain > 2.0f - 5e-5f && result.gain < 2.0f + 5e-5f);
    assert(result.bias > 1.0f - 5e-5f && result.bias < 1.0f + 5e-5f);
}
