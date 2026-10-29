// The CUDA active-mask pass must not re-ballot at the merge of a region that
// every lane leaves through its merge block (shader-slang/slang#14016).
//
// The `?:` and `||` below lower to `if`/`else` regions whose arms both end at
// the merge block, the loop is only left through its `break` label (where
// lanes leave after different numbers of iterations), and every `case` of the
// `switch` ends with `break`. Every lane that enters one of these regions also
// leaves it at its merge block, so the mask after the merge is the mask on
// entry, and the `WaveActiveSum` needs no ballot besides the initial mask of
// the kernel.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -O3 -validate-ir
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set inputBuffer = ubuffer(data=[1 2 3 4 5 6 7 8], stride=4)
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)

StructuredBuffer<uint> inputBuffer;
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    bool odd = (lane & 1) != 0;
    uint a = odd ? inputBuffer[lane] * 3 : inputBuffer[lane] + 10;
    bool flag = odd || inputBuffer[lane] == 3;

    uint acc = 0;
    for (uint i = 0; i < lane; i++)
        acc += inputBuffer[i];

    switch (lane % 3)
    {
    case 0:
        acc += 100;
        break;
    case 1:
        acc += 200;
        break;
    default:
        acc += 300;
        break;
    }

    uint total = WaveActiveSum(flag ? a : 0);
    outputBuffer[lane] = total + acc;
}

// CUDA-LABEL: __global__ void computeMain(
// CUDA: __ballot_sync(4294967295U, true)
// CUDA-NOT: __ballot_sync(

// a = 11 6 13 12 15 18 17 24; lanes 1, 2, 3, 5 and 7 sum 6 + 13 + 12 + 18 + 24 = 73.
// acc = (1 + ... + lane) + 100, 200 or 300 for lane % 3 = 0, 1 or 2.
// CHECK: 173
// CHECK-NEXT: 274
// CHECK-NEXT: 376
// CHECK-NEXT: 179
// CHECK-NEXT: 283
// CHECK-NEXT: 388
// CHECK-NEXT: 194
// CHECK-NEXT: 301
