//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6d3d353f7acb30664cc90831179bf06d5f613a7e643d2f70a23548558a35ee35
//META: purpose=The interpolation modifiers `nointerpolation`, `noperspective`, `linear` and `pervertex` share one conflict group, so two of them on one field is rejected (E31202).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the third named multi-member conflict group, which is the largest of
// the three and the one whose members read most like independent qualifiers.
// `nointerpolation` and `linear` describe incompatible rasterizer behaviour, so
// the group rule has to reject the pair rather than let the later one win.
// CHECK pins E31202 and the message naming `linear` against `nointerpolation`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct V
{
    nointerpolation linear float4 c : COLOR;
//CHECK: E31202
//CHECK: modifier 'linear' is redundant or conflicting with existing modifier 'nointerpolation'
}

void main()
{
}
