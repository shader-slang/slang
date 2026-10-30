// A custom derivative may use thread-group-shared memory directly, with its own barriers, because
// autodiff calls it as written instead of transforming its body. This test follows the pattern of
// `loadFloat` in the slang-playground `gsplat2d-diff.slang` demo: the primal reads a value that
// every invocation shares, and the custom backward derivative sums each invocation's gradient with
// a barrier-separated reduction in shared memory before one invocation stores the total.
//
// Invocation `tid` differentiates `w * x * x` at `x = tid + 1` with `w = 2`. Its derivative with
// respect to `x` is `2 * w * x`, giving 4, 8, 12 and 16. Its derivative with respect to `w` is
// `x * x`, and the reduction sums those to 1 + 4 + 9 + 16 = 30.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

groupshared float weights[4];
groupshared float reductionBuffer[4];

[BackwardDerivative(loadWeight_bwd)]
float loadWeight(uint idx, uint tid)
{
    return weights[idx];
}

void loadWeight_bwd(uint idx, uint tid, float dOut)
{
    reductionBuffer[tid] = dOut;
    GroupMemoryBarrierWithGroupSync();

    for (uint stride = 2; stride > 0; stride /= 2)
    {
        if (tid < stride)
            reductionBuffer[tid] += reductionBuffer[tid + stride];
        GroupMemoryBarrierWithGroupSync();
    }

    if (tid == 0)
        outputBuffer[4 + idx] = reductionBuffer[0];
}

[Differentiable]
float loss(float x, uint tid)
{
    float w = loadWeight(0, tid);
    return w * x * x;
}

[numthreads(4, 1, 1)]
void computeMain(uint tid: SV_GroupIndex)
{
    weights[tid] = 2.0 + float(tid);
    GroupMemoryBarrierWithGroupSync();

    var x = diffPair(float(tid + 1));
    bwd_diff(loss)(x, tid, 1.0);
    outputBuffer[tid] = x.d;
}

// CHECK: 4.0
// CHECK-NEXT: 8.0
// CHECK-NEXT: 12.0
// CHECK-NEXT: 16.0
// CHECK-NEXT: 30.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
