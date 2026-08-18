//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#two-stage-parsing
//META: doc_section_digest=ddfe5f62e9b1084664029d4e0b1a2903b23b23769c16d58583a7289396a20747
//META: purpose=Body-parse-time checker knowledge decides `<`: the same token shape reads as a generic application when the base names a generic and as a comparison when the base is an ordinary local.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies why bodies are re-parsed after checking rather than during the
// decl-parse stage. Both statements below have the identical token shape
// `name < name > ( name )`, and in both cases the token after the matching `>`
// is `(`, which is a generic-follower — so a purely syntactic rule would have
// to read both as generic applications. The first resolves to 7 only if
// `pick<int>(...)` was read as a generic call; the second resolves to 0 only if
// `a < b > (c)` was read as `(a < b) > c`, which requires knowing that the local
// `a` is not a generic. The pair therefore isolates the semantic signal the
// body-parse stage supplies.

//TEST:INTERPRET(filecheck=CHECK):

T pick<T : IComparable>(T x, T y) { return x > y ? x : y; }

void main()
{
    int a = 1;
    int b = 2;
    int c = 3;

    int viaGeneric = pick<int>(3, 7);
    int viaComparison = (a < b > (c)) ? 1 : 0;

    // CHECK: generic=7 comparison=0
    printf("generic=%d comparison=%d\n", viaGeneric, viaComparison);
}
