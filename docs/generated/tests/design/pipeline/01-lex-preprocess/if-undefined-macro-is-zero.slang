//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=81abdf54ca8e6b727b5142e63c4757374b3f566da88d8b9672ba7862e21bd136
//META: purpose=Boundary probe of `#if`: an undefined identifier in the expression is treated as `0` per the standard C/HLSL `#if` semantics referenced by the doc.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `NEVER_DEFINED` is not introduced anywhere. The preprocessor must
// substitute `0` for the bare identifier inside a `#if` expression,
// so the active branch must be `#else`.

//TEST:INTERPRET(filecheck=CHECK):

#if NEVER_DEFINED
static const int picked = 1;
#else
static const int picked = 0;
#endif

void main()
{
    // CHECK: picked=0
    printf("picked=%d\n", picked);
}
