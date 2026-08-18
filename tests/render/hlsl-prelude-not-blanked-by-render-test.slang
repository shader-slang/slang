// Regression test for shader-slang/slang#12462.
//
// slang-test may share one GlobalSession across the tests it runs, so a prelude
// mutation in the render-test (`-cpu` COMPARE_COMPUTE) path is observable by a
// later `-target hlsl` compile on the same session. The two directives below run
// in that order within one file: the first exercises the render-test path, the
// second asserts render-test did not blank the shared session's HLSL prelude.
// An `#include "…nvHLSLExtns.h"` line appears in every non-blank HLSL prelude the
// harness can install — the compiler default (guarded by
// `#ifdef SLANG_HLSL_ENABLE_NVAPI`, from prelude/slang-hlsl-prelude.h), and the
// NVAPI overrides installed by render-test and slang-test (which use an absolute
// path) — but is absent from a blanked prelude, so the `{{.*}}` pattern below is
// the marker for "prelude still present".

//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = 1;
}

//BUF: 1
//HLSL: #include "{{.*}}nvHLSLExtns.h"
