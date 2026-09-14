//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=c08befb0cda99dc013ab0ceb9ba3b786beb2359fadcd06a399a784137456d81d
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
