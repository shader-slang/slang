//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=cb681a5a5428e160c2311cbf091e8eda34ecba068f868b779a464083304bb62f
//META: purpose=A `where nonempty(Pack)` clause constrains a type pack to have at least one element and is satisfied by a non-empty argument list.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `nonempty(...)` constraint spelling on a variadic generic. The
// generic is called with two arguments and returns the pack length plus a
// marker; the CHECK asserts 102, so the constraint parsed as a pack constraint
// and was satisfied rather than rejecting the call.

//TEST:INTERPRET(filecheck=CHECK):

int nonEmptyLen<each T>(expand each T args) where nonempty(T)
{
    return countof(T) + 100;
}

void main()
{
    //CHECK: 102
    printf("%d\n", nonEmptyLen(1, 2));
}
