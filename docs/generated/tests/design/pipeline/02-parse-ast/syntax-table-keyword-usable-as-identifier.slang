//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#syntax-as-declaration
//META: doc_section_digest=96201e4a01337de99d9651244ec405bb74f7aa94360c94d6d62426600a323482
//META: purpose=Modifier and attribute keywords are identifiers bound to syntax in the active environment, not lexer-level reserved words, so `nointerpolation` and `unroll` can also be ordinary local variable names.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "most keywords are not reserved words" claim from the opposite
// direction to the usual modifier tests: instead of showing that
// `nointerpolation` works as a modifier, it shows the lexer never marked the
// spelling as special. Both names are used as plain locals, declared and then
// read; if either were a reserved token the declarations would not parse. The
// printed 10 is 7 + 3, so both bindings were also readable afterwards.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int nointerpolation = 7;
    int unroll = 3;
    // CHECK: v=10
    printf("v=%d\n", nointerpolation + unroll);
}
