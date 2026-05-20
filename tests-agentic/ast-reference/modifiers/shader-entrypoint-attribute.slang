//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#stage-specific-entry-point-attributes
//META: doc_section_digest=5332741d22bd1f66cc006924c76c4414130d880c08abe1915fb243255799a71b
//META: purpose=`[shader("compute")]` marks a function as a compute entry point selectable without -entry.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `EntryPointAttribute` carries the pipeline stage name. A function
// carrying `[shader("compute")]` is found as a compute entry point;
// the emitted HLSL contains the `numthreads` attribute placed on
// that function.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void csMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = 7;
}

//CHECK: numthreads
//CHECK: csMain
