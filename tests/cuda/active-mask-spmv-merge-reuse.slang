// The CUDA active-mask pass must keep only the ballots that a later wave op
// needs in a segmented warp reduction like the one reported on
// shader-slang/slang#14016: one WaveActiveBallot, then WaveReadLaneAt in an
// [unroll] loop. The `?:` and `||` before the loop lower to `if`/`else` regions
// with only normal exits, so their merges reuse the mask on entry. What
// remains is the initial mask and the loop's four exit ballots: the `true` and
// `false` exits of the `if` that `iter < 32` is lowered to, and the `false`
// (break) and `true` (continue) exits of each iteration.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -O3 -validate-ir

StructuredBuffer<uint> rows;
StructuredBuffer<uint> cols;
StructuredBuffer<float> vals;
StructuredBuffer<float> x;
RWStructuredBuffer<float> y;

[shader("compute")]
[numthreads(256, 1, 1)]
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
