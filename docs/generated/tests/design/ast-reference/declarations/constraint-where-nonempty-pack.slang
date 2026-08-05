//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=3c3b338a49679f1b453ba7815d8deac7d31be7956bc0fe69e33cdab3863157ae
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
