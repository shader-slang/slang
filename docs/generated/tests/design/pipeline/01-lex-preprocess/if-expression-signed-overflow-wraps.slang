//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=81abdf54ca8e6b727b5142e63c4757374b3f566da88d8b9672ba7862e21bd136
//META: purpose=Boundary probe of arithmetic in `#if`: `(INT_MAX + 1)` wraps to a negative value (observed behavior).
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `2147483647 + 1` in a `#if` expression overflows the signed result.
// The observed behavior is wrap-to-negative; the `< 0` branch must
// therefore be taken. This probes the arithmetic evaluator used
// inside the preprocessor.

//TEST:INTERPRET(filecheck=CHECK):

#if (2147483647 + 1) < 0
static const int picked = 1;
#else
static const int picked = 0;
#endif

void main()
{
    // CHECK: picked=1
    printf("picked=%d\n", picked);
}
