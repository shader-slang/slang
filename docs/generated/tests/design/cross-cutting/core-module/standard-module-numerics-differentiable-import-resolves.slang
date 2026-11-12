//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:58:48+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#standard-modules
//META: doc_section_digest=19b72d3c97699e7ac3c4b56c42266ecaaaecfdddcb2eb6781aafb43d9bc7fb56
//META: purpose=slang.numerics.differentiable, an artifact nested one level deeper than numerics.slang-module, is a client-facing entry point that resolves by import path once experimental features are enabled.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states that nesting under slang/numerics/ does not imply
// privacy: differentiable.slang-module is imported as
// slang.numerics.differentiable. A successful HLSL emit with
// -experimental-feature proves the import resolved; the CHECK pins that
// the entry point was still emitted.

//TEST:SIMPLE(filecheck=HLSL):-experimental-feature -target hlsl -entry computeMain -stage compute

import slang.numerics.differentiable;

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1.0;
}

// HLSL: void computeMain
