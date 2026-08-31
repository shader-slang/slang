//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A TypeDefDecl (`typedef X Y;`) introduces a name for an existing type; the alias is usable wherever the original type would be.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `typedef` name denotes the same type as its target rather
// than a distinct one. A value declared with the alias is passed to a function
// whose parameter is spelled with the original type; the CHECK asserts the
// round-tripped value, which requires the call to type-check.

//TEST:INTERPRET(filecheck=CHECK):

typedef int Number;

int identity(int x) { return x; }

void main()
{
    Number n = 99;
    //CHECK: 99
    printf("%d\n", identity(n));
}
