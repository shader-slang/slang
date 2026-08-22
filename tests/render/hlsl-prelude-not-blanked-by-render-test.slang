// Regression test for shader-slang/slang#12462.
//
// The two directives below run in order within one file: the first exercises the
// render-test path (`-cpu` COMPARE_COMPUTE), the second asserts render-test did
// not blank the HLSL prelude on the session they share.
//
// This guard is meaningful only when both directives run against the SAME session
// — the property the fix concerns. That holds under slang-test's default in-process
// mode (`SpawnType::UseSharedLibrary` hands the single `context->getSession()` to
// every tool's inner-main) and under `-use-test-server` (one session per server),
// which are the modes CI uses. Under the isolated spawn modes `-use-exe` and
// `-use-fully-isolated-test-server` each tool gets a fresh session, so the second
// directive sees the default prelude and this check passes trivially — the test
// does not detect a reintroduced blanking there.
//
// The marker: an `#include "…nvHLSLExtns.h"` line appears in every non-blank HLSL
// prelude the harness can install — the compiler default (guarded by
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
