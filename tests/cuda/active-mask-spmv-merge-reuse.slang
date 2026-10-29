// The CUDA active-mask pass must keep only the ballots that a later wave op
// needs in a segmented warp reduction like the one reported on
// shader-slang/slang#14016: one WaveActiveBallot, then WaveReadLaneAt in an
// [unroll] loop. The `?:` and `||` before the loop lower to `if`/`else` regions
// with only normal exits, so their merges reuse the mask on entry. What
// remains in the kernel body (the user's `WaveActiveBallot` is emitted in a
// helper function) is the initial mask and the loop's four exit ballots: the
// `true` and `false` exits of the `if` that `iter < 32` is lowered to, and the
// `false` (break) and `true` (continue) exits of each iteration.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -O3 -validate-ir
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set rows = ubuffer(data=[0 0 0 0 1 1 1 1 2 2 2 2 3 3 3 3 4 4 4 4 5 5 5 5 6 6 6 6 7 7 7 7 8 8 8 8 9 9 9 9 10 10 10 10 11 11 11 11 12 12 12 12 13 13 13 13 14 14 14 14 15 15 15 15], stride=4)
//TEST_INPUT: set cols = ubuffer(data=[0 2 3 4 1 3 4 5 2 4 5 6 3 5 6 7 4 6 7 8 5 7 8 9 6 8 9 10 7 9 10 11 8 10 11 12 9 11 12 13 10 12 13 14 11 13 14 15 12 14 15 16 13 15 16 17 14 16 17 18 15 17 18 19], stride=4)
//TEST_INPUT: set vals = ubuffer(data=[1.0 8.0 4.0 11.0 7.0 3.0 10.0 6.0 2.0 9.0 5.0 1.0 8.0 4.0 11.0 7.0 3.0 10.0 6.0 2.0 9.0 5.0 1.0 8.0 4.0 11.0 7.0 3.0 10.0 6.0 2.0 9.0 5.0 1.0 8.0 4.0 11.0 7.0 3.0 10.0 6.0 2.0 9.0 5.0 1.0 8.0 4.0 11.0 7.0 3.0 10.0 6.0 2.0 9.0 5.0 1.0 8.0 4.0 11.0 7.0 3.0 10.0 6.0 2.0], stride=4)
//TEST_INPUT: set x = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 1.0 2.0 3.0 4.0 5.0 1.0 2.0 3.0 4.0 5.0 1.0 2.0 3.0 4.0 5.0], stride=4)
//TEST_INPUT: set y = out ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0], stride=4)
//TEST_INPUT: set nnz = 50

StructuredBuffer<uint> rows;
StructuredBuffer<uint> cols;
StructuredBuffer<float> vals;
StructuredBuffer<float> x;
RWStructuredBuffer<float> y;

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uniform uint nnz)
{
    uint i = tid.x;
    uint lane = WaveGetLaneIndex();
    bool valid = i < nnz;
    uint j = valid ? i : 0;
    uint row = rows[j];
    uint col = cols[j];
    float v = valid ? vals[j] * x[col] : 0;
    bool isBoundary = !valid || row == col || lane == 0;
    uint mark = reversebits(WaveActiveBallot(isBoundary).x);
    [unroll]
    for (uint iter = 1; iter < 32; iter <<= 1)
    {
        float o = WaveReadLaneAt(v, (lane + iter) & 31);
        uint segMask = mark << (lane + 1);
        if (lane + iter < 32 && (segMask >> (32 - iter)) == 0)
            v += o;
    }
    if (isBoundary && valid)
        y[row] = v;
}

// CUDA-LABEL: __global__ void computeMain(
// CUDA: __ballot_sync(4294967295U, true)
// CUDA-COUNT-4: __ballot_sync(
// CUDA-NOT: __ballot_sync(

// Rows 0-12 each start at a diagonal entry (every fourth nonzero); lanes 50-63 are invalid.
// CHECK: 96.000000
// CHECK-NEXT: 82.000000
// CHECK-NEXT: 58.000000
// CHECK-NEXT: 79.000000
// CHECK-NEXT: 61.000000
// CHECK-NEXT: 68.000000
// CHECK-NEXT: 90.000000
// CHECK-NEXT: 80.000000
// CHECK-NEXT: 49.000000
// CHECK-NEXT: 118.000000
// CHECK-NEXT: 73.000000
// CHECK-NEXT: 65.000000
// CHECK-NEXT: 36.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
