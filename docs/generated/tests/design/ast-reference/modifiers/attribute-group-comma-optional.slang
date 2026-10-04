//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#modifiers-vs-attributes
//META: doc_section_digest=0be0f22a51918f9fc5f00ab42a13ab6eb20120ba83f03c20f196cfda4fc6178d
//META: purpose=Several attributes may share one bracket group and the comma between them is optional.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both halves of the shared-group rule: two attributes live in one
// `[...]` group, and they are separated by whitespace with no comma. If the
// parser required a separator, or bound only the first entry in the group, the
// stage marker or the workgroup size would be missing; the CHECK-DAG lines
// assert the emitted HLSL carries both.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl

RWStructuredBuffer<int> outBuf;

[shader("compute") numthreads(4, 1, 1)]
void csMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

//CHECK-DAG: numthreads(4, 1, 1)
//CHECK-DAG: csMain
