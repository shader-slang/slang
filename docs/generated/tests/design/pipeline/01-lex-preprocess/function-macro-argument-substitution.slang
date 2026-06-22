//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=5bd3f7fd4d4aec36189e915e3bcaded5ad098fe089b37ce5114d8bfba37a75a3
//META: purpose=Function-style macros allocate a fresh environment that maps each parameter name to its argument, so the argument tokens replace the parameter at expansion.
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
