//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies that the optional second argument of `sizeof` selects the data layout, so the same type reports different sizes per layout.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The size operand takes an optional layout selector; without it the scalar
// layout is used. A struct of two `float3` fields has a different size under
// each layout because of their differing alignment rules, so the three CHECKs
// asserting distinct values prove the selector was honoured rather than
// ignored.

//TEST:INTERPRET(filecheck=CHECK):

struct S { float3 a; float3 b; }

static const int scalarSize = sizeof(S);
static const int std140Size = sizeof(S, Std140DataLayout);
static const int std430Size = sizeof(S, Std430DataLayout);

void main()
{
    // CHECK: scalar=24
    printf("scalar=%d\n", scalarSize);
    // CHECK: std140=32
    printf("std140=%d\n", std140Size);
    // CHECK: std430=28
    printf("std430=%d\n", std430Size);
}
