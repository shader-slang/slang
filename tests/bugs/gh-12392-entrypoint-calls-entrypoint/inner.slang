// Companion module for gh-12392: a function that is itself an entry point.
// Imported and called by `outer.slang`'s entry point, which is the trigger for the crash.
module inner;

[shader("compute")]
[numthreads(32, 1, 1)]
public void k(uint tid, RWStructuredBuffer<float> buf)
{
    buf[tid] = tid * 2.0;
}
