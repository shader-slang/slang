//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A GenericValuePackParamDecl (`let each N`) is a variadic value-pack parameter whose elements are usable one at a time.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `let each N : int` declares a pack of compile-time values
// rather than a single one. The body expands the pack into an accumulating
// sum; the CHECK asserts the total of the three supplied arguments, so each
// element must have been visited.

//TEST:INTERPRET(filecheck=CHECK):

int sumPack<let each N : int>()
{
    int s = 0;
    expand s += (each N);
    return s;
}

void main()
{
    //CHECK: 6
    printf("%d\n", sumPack<1, 2, 3>());
}
