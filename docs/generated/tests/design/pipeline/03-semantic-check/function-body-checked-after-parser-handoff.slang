//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#two-pass-interaction-with-the-parser
//META: doc_section_digest=256a93a6f5809caaf2388b80d4fd482fa44b1f1872e9a0e6becde5c2caebda36
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
