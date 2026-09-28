//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#glsl-module
//META: doc_section_digest=72709ab05fe0048310a93cb48fcbdc8ad10b25203945acdda232920d909f6344
//META: purpose=An explicit import glsl retrieves the GLSL builtin module, after which its aliases resolve and lower to the target's own spelling.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the GLSL module is reachable by naming it in an import,
// which is the retrieval path for the builtin module the session
// holds. This file differs from the negative companion only by that
// one import line, so the CHECKs -- the emitted HLSL vector spelling
// and the folded sum -- pin that the import is what brought `vec3`
// into scope rather than any implicit visibility.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute

import glsl;

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    vec3 v = vec3(float(tid.x), 1.0, 2.0);
    outBuf[tid.x] = v.x + v.y;
}

// HLSL: void computeMain(
// HLSL: float(tid_{{[0-9]+}}.x) + 1.0f
