// Companion to sum-vector-matrix-gated.slang for issue #11917.
//
// When the module contains no `kIROp_SumVectorElements` / `kIROp_SumMatrixElements`,
// `lowerSumVectorMatrixInsts` has nothing to lower. This plain compute shader does no autodiff, so
// the transpose pass never runs and no sum-reduction IR is produced; the gate flag stays false.
// Whether the pass is actually *skipped* here rather than run as a no-op is a compile-time-only
// property that is not observable in emitted output; that gated-vs-ungated equivalence is shown by
// the byte-identical revert-drill described in the PR, not by this test. This is a baseline that
// must still compile and compute the plain result unchanged.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float3 v = float3(2.0f, 3.0f, 4.0f);
    // CHECK: 9.0
    outputBuffer[0] = v.x + v.y + v.z;
}
