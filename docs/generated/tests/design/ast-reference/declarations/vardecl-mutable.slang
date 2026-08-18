//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A VarDecl is an ordinary mutable variable; its value can be assigned and re-read.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the binding a `VarDecl` introduces is mutable: the local
// declared with an initializer accepts a later assignment. The CHECK asserts
// the second value, which only prints if the assignment took effect rather
// than being rejected or discarded.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 1;
    x = 42;
    //CHECK: 42
    printf("%d\n", x);
}
