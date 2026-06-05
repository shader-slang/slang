// used-uniforms-matrix.slang

// Tests that a matrix element read widens to the whole matrix. Column
// major storage makes per element byte ranges discontiguous, so the pass
// deliberately does not narrow matrix subscripts: reading a single
// mat[i][j] records the entire matrix region. A sibling field placed
// after the matrix is never read and must stay out of the recorded
// ranges, confirming the widening is bounded to the matrix rather than
// the whole buffer. No other used-uniforms test exercises a matrix, so
// this is the only pin on that widening rule.

//TEST:REFLECTION:-stage compute -entry main -target hlsl

cbuffer CB
{
    float4x4 mat;
    float4   unused;
}

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = mat[0][0];
}
