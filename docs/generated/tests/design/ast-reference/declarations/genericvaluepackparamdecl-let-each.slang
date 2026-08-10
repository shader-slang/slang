//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
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
