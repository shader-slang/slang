//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A VarDecl's initializer expression populates the variable's storage before first use.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a variable declaration's initializer expression reaches
// runtime rather than being dropped in favour of default zero-initialization.
// The CHECK asserts the literal 17, which is only observable if the
// declaration carried its initializer through checking and lowering.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 17;
    //CHECK: 17
    printf("%d\n", x);
}
