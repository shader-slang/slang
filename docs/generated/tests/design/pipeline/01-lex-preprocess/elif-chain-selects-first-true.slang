//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=c3ad01b7a04e24b43f66e2d017e85a491e5a0fbbf99a28a1c828245742a23613
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
