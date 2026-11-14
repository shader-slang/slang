//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=3c3b338a49679f1b453ba7815d8deac7d31be7956bc0fe69e33cdab3863157ae
//META: purpose=A `where countof(Pack) == N` clause ties a value parameter to a type pack's element count.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the pack-count constraint spelling. The generic is specialized with
// an explicit `N` of 3 and called with exactly three pack arguments; the body
// returns a value derived from `N`, and the CHECK asserts it, so the
// constraint must have accepted the pairing of count and value.

//TEST:INTERPRET(filecheck=CHECK):

int countTagged<let N : int, each T>(expand each T args) where countof(T) == N
{
    return N * 10;
}

void main()
{
    //CHECK: 30
    printf("%d\n", countTagged<3>(1, 2, 3));
}
