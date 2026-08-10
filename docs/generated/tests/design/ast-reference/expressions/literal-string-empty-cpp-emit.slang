//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=1e8f144399f6073f93949619c72c2230cfb4349def0ab49ac5088e68f85aa4ab
//META: purpose=Verifies that an empty StringLiteralExpr `""` lowers and appears as an empty string literal in emitted C++ output.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary for StringLiteralExpr: an empty string. The
// emitted C++ printf call must contain `""` verbatim.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    printf("");
    // CHECK: printf("")
}
