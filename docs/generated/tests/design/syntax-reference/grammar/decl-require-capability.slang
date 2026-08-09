//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#user-defined-syntax
//META: doc_section_digest=7b5f8dd8046f56712186bea6498104c3a0faf4b662ee04fc92a8939c967b50e7
//META: purpose=`RequireCapabilityDecl` parses as `'__require_capability' CapabilityName ';'` with no parentheses around the name.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the module-scope capability declaration takes a bare
// capability atom terminated by `;` — no parentheses, unlike the
// statement-level `__requireCapability(...)` spelling. The declaration
// sits before the entry point, so if the parser stopped short of the
// `;` the following function would not be reached; the CHECK on the
// emitted HLSL entry point is what proves the whole file parsed.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain

__require_capability sm_5_0;

RWStructuredBuffer<int> outputBuffer;

// CHECK: void computeMain()
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 7;
}
