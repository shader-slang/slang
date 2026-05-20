//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=preprocessor: the preprocessor handles `#define` macro substitution on the lexer's token stream before parsing, so an identifier defined as a constant is replaced with that constant in later uses.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "preprocessor" glossary entry: `#define ANSWER 42`
// causes uses of `ANSWER` to be substituted with `42` before the
// parser runs, so the printed value is 42.

//TEST:INTERPRET(filecheck=CHECK):

#define ANSWER 42

void main()
{
    int v = ANSWER;
    // CHECK: pp=42
    printf("pp=%d\n", v);
}
