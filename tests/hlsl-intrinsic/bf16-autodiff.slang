//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type -render-features bfloat16 -skip-spirv-validation
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -render-features bfloat16

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Verify BFloat16 auto-diff works (forward and backward).
// BFloat16 is now IDifferentiable with Differential = BFloat16.

// f(x) = x * x + x
// f'(x) = 2*x + 1
// At x=3: f(3) = 12, f'(3) = 7

// Forward diff
// CHECK: 120
// CHECK: 70

// Backward diff
// CHECK: 120
// CHECK: 70

[Differentiable]
BFloat16 quadratic(BFloat16 x)
{
    return x * x + x;
}

[numthreads(1,1,1)]
void computeMain()
{
    BFloat16 x = BFloat16(3.0);

    // Forward differentiation
    var fwdPair = diffPair(x, BFloat16(1.0));
    let fwdResult = fwd_diff(quadratic)(fwdPair);
    outputBuffer[0] = int(float(fwdResult.p) * 10.0); // f(3) = 12 -> 120
    outputBuffer[1] = int(float(fwdResult.d) * 10.0); // f'(3) = 7 -> 70

    // Backward differentiation
    var bwdPair = diffPair(x);
    bwd_diff(quadratic)(bwdPair, BFloat16(1.0));
    let bwdPrimal = quadratic(x);
    outputBuffer[2] = int(float(bwdPrimal) * 10.0);  // f(3) = 12 -> 120
    outputBuffer[3] = int(float(bwdPair.d) * 10.0);  // f'(3) = 7 -> 70
}
