//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=85307207e3b8f9c4a6840f45c1896c59f8f18e3bce3e9753d2b831e1d7ea443a
//META: purpose=A call to a `[NoDiscard]` function whose result is consumed compiles cleanly and runs — the attribute only errors in result-discarding contexts.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the positive path of the [NoDiscard] claim: the doc scopes
// the error to result-discarding contexts, so binding the result to a
// variable must compile and produce the value. The printed value
// proves the attributed function was callable and its result usable.

//TEST:INTERPRET(filecheck=CHECK):

[NoDiscard]
int produce()
{
    return 42;
}

void main()
{
    int v = produce();
    // CHECK: v=42
    printf("v=%d\n", v);
}
