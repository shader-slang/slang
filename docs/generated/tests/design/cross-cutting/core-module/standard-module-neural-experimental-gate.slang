//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#standard-modules
//META: doc_section_digest=d64fe737834983844ed7939a74943fc37884d9afbd7fcd1aa49239095501c0e5
//META: purpose=The neural standard module is declared [ExperimentalModule], so import slang.neural is refused unless experimental features are enabled.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the neural standard module is both discoverable by its
// qualified import path and gated by the [ExperimentalModule]
// attribute its module declaration carries. Compiling without the
// experimental opt-in produces E00104 naming the resolved module path
// 'slang/neural' — the path proves the qualified name was mapped to
// the shipped subdirectory, and the code proves the attribute was
// honoured rather than the module simply being missing.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

import slang.neural;
/*CHECK:
       ^^^^^ 'slang/neural' is an experimental module, need to enable '-experimental-feature' to load this module
*/

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 0;
}
