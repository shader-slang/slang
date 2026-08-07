//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
//META: purpose=A GenericTypePackParamDecl (`each T`) is a variadic type-pack parameter whose element count is queryable inside the generic.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `each T` declares one parameter standing for a whole pack of
// types. The generic is called with three arguments of different types and
// returns the pack's element count; the CHECK asserts 3, which is only right
// if all three argument types were bound into one pack parameter.

//TEST:INTERPRET(filecheck=CHECK):

int packLen<each T>(expand each T args) { return countof(T); }

void main()
{
    //CHECK: 3
    printf("%d\n", packLen(1, 2.0f, 3u));
}
