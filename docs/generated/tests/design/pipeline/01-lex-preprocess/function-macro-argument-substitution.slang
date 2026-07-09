//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=cf4f1b505a333d6ccc1a3cb482330c4dde4668bd530a4d0fe4905412ffd9448a
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
