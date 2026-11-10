// Companion module for `outer.slang`: a compute entry point that `outer.slang` imports and calls as
// an ordinary function.
module inner;

[shader("compute")]
[numthreads(32, 1, 1)]
public void k(uint tid, RWStructuredBuffer<float> buf)
{
    buf[tid] = tid * 2.0;
}
