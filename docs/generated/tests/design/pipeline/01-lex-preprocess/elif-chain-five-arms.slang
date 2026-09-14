//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=81abdf54ca8e6b727b5142e63c4757374b3f566da88d8b9672ba7862e21bd136
//META: purpose=Stress probe of `#elif`: in a 5-arm `#if`/`#elif*4`/`#else` chain, only the matching arm runs and the others are inactive.
//META: intent=stress
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Long `#elif` chain. With MODE = 4, only the fourth arm should
// become active; every other arm — including the `#else` — must be
// inactive.

//TEST:INTERPRET(filecheck=CHECK):

#define MODE 4

#if MODE == 1
static const int picked = 1;
#elif MODE == 2
static const int picked = 2;
#elif MODE == 3
static const int picked = 3;
#elif MODE == 4
static const int picked = 4;
#elif MODE == 5
static const int picked = 5;
#else
static const int picked = 0;
#endif

void main()
{
    // CHECK: picked=4
    printf("picked=%d\n", picked);
}
