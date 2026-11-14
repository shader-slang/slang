//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
//META: purpose=An IntegerLiteralExpr's suffixType drives overload selection: 1u selects the uint overload, 1 selects the int overload.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an integer literal's suffix selects the literal's type. Two
// overloads that differ only in int vs uint act as a probe: the CHECK asserts
// the `u`-suffixed literal picked the uint overload and the bare literal
// picked the int one.

//TEST:INTERPRET(filecheck=CHECK):

int which(int v)  { return 1; }
int which(uint v) { return 2; }

void main()
{
    // CHECK: int=1
    printf("int=%d\n", which(1));
    // CHECK: uint=2
    printf("uint=%d\n", which(1u));
}
