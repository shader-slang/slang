//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=73b0a54aa16472868adbc586d5469e6a6d226c42e5fd1a1ff6720178eede46b1
//META: purpose=A `packoffset()` binding on an entry-point parameter is reported as an unsupported-and-ignored modifier (E38010) unconditionally.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fourth member of the ignored-binding-modifier family, and the
// third of the three that are diagnosed with no gating at all. The parameter is
// a plain `uniform int`, so nothing about the type or the target could suppress
// the report; if it fires here it fires whenever the modifier appears in this
// position. CHECK pins E38010 and the message naming the keyword and parameter.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0): -target hlsl

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform int v: packoffset(c0.x), uniform RWStructuredBuffer<int> buf)
//CHECK: E38010
//CHECK: keyword 'packoffset' on parameter 'v'
{
    buf[0] = v;
}
