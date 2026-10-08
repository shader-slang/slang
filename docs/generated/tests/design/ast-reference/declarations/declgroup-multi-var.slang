//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A DeclGroup wraps multiple declarations parsed as one group (`int a, b;`); each name is independently visible.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a comma-separated declaration introduces two independent
// names rather than one. Each name is assigned a different value and the
// CHECK asserts their sum, which is only correct if both bindings exist
// separately.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a, b;
    a = 3;
    b = 5;
    //CHECK: 8
    printf("%d\n", a + b);
}
