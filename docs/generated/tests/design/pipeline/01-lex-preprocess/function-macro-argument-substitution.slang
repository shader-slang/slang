//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=cfe8fdce64ff12d1e1dbd2e4845beaf13fd1ba25fc9f4115e0139eab627320e7
//META: purpose=A parameter reference in a macro body is compiled to a parameter op, and invocation replays the matching argument's token range by index, so the argument tokens replace the parameter at expansion.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a function-style macro's arguments are substituted
// into the body. `DOUBLE(7)` must expand to `((7) * 2)` so the
// printed value is 14. If parameter substitution were broken, the
// expansion would still reference `x` and the test would not
// compile.

//TEST:INTERPRET(filecheck=CHECK):

#define DOUBLE(x) ((x) * 2)

void main()
{
    // CHECK: doubled=14
    printf("doubled=%d\n", DOUBLE(7));
}
