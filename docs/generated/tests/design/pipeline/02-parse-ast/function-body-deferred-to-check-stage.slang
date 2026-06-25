//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#two-stage-parsing
//META: doc_section_digest=533454eadd55380449930cf39d9145725c95943e6be5773e59633f3f506b93cf
//META: purpose=Function bodies are captured at decl-stage as UnparsedStmt and re-parsed in body-parse mode, so a body that references a generic call (which depends on body-stage `<` disambiguation) still parses and runs.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The body of `main` contains `pickMax<int>(3, 7)`. The `<` after
// `pickMax` is the disambiguation case that the doc says is resolved
// at body-parse time with help from the semantic checker. A clean run
// confirms the two-stage scheme reached the body, re-parsed it, and
// committed to the generic interpretation.

//TEST:INTERPRET(filecheck=CHECK):

T pickMax<T : IComparable>(T a, T b)
{
    if (a > b) return a;
    return b;
}

void main()
{
    int m = pickMax<int>(3, 7);
    // CHECK: m=7
    printf("m=%d\n", m);
}
