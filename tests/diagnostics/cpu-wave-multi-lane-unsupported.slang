// On the CPU targets every thread is a wave of one lane. Wave intrinsics that need several lanes,
// such as the quad operations, are still rejected there.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target cpp -entry computeMain -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^^^^^^^^^^^ unavailable features in entry point
     ^^^^^^^^^^^ entrypoint 'computeMain' uses features that are not available in 'compute' stage for 'cpp' compilation target.
*/
{
    outBuf[tid.x] = QuadReadAcrossX(float(tid.x));
/*CHECK:
                    ^^^^^^^^^^^^^^^ see using of 'QuadReadAcrossX'
*/
}
