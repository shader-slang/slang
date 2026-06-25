//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#two-pass-interaction-with-the-parser
//META: doc_section_digest=f3d484a3a8cd389ef2580b52eec5f829b230f82ed6207c049ca30a93d731eb8a
//META: purpose=Function bodies are left as UnparsedStmt by the parser and checked on demand; a well-formed body parses and runs.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says the parser hands function bodies to the checker as
// UnparsedStmt nodes which are parsed on demand. Verify the user-
// visible consequence: a body containing locals, branches, and a
// call executes correctly.

//TEST:INTERPRET(filecheck=CHECK):

int classify(int n)
{
    if (n < 0) return -1;
    else if (n == 0) return 0;
    else return 1;
}

void main()
{
    // CHECK: c=-1 0 1
    printf("c=%d %d %d\n", classify(-5), classify(0), classify(7));
}
