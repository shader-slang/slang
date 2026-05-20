//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=3d8d27a59aa70cbff18bfb217be0513b6ff9ff565fb5ebcb7298879f0a47f2fd
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
