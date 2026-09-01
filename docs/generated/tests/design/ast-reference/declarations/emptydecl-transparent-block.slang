//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A `__transparent_block { ... }` parses its contents into the enclosing container and itself becomes an empty declaration.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a transparent block does not introduce a scope of its own. A
// function is declared inside the block and called from module-scope code
// outside it; the CHECK asserts that function's result, which requires the
// declaration to have landed in the enclosing container.

//TEST:INTERPRET(filecheck=CHECK):

__transparent_block { int hidden() { return 17; } }

void main()
{
    //CHECK: 17
    printf("%d\n", hidden());
}
