//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A TypeDefDecl introduces a typedef-style name for an existing type; the alias is usable wherever the original type would be.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of TypeDefDecl (`typedef X Y;`): the
// declared name `Number` refers to the same type as `int`, and a
// function taking `int` accepts a value typed as `Number`.

//TEST:INTERPRET(filecheck=CHECK):

typedef int Number;

int identity(int x) { return x; }

void main()
{
    Number n = 99;
    //CHECK: 99
    printf("%d\n", identity(n));
}
