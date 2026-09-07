//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=81abdf54ca8e6b727b5142e63c4757374b3f566da88d8b9672ba7862e21bd136
//META: purpose=The preprocessor supports `#elif` as part of the standard C/HLSL conditional set; in a chain it selects the first arm whose expression evaluates to non-zero.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A 3-way `#if`/`#elif`/`#else` chain. With `MODE` defined to 2,
// only the second arm should be active; the first arm and the
// `#else` arm should be discarded.

//TEST:INTERPRET(filecheck=CHECK):

#define MODE 2

#if MODE == 1
static const int picked = 11;
#elif MODE == 2
static const int picked = 22;
#else
static const int picked = 99;
#endif

void main()
{
    // CHECK: picked=22
    printf("picked=%d\n", picked);
}
