//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=c3ad01b7a04e24b43f66e2d017e85a491e5a0fbbf99a28a1c828245742a23613
//META: purpose=Boundary probe of `#if`: `0xFFFFFFFF` (uint32 MAX) evaluates as non-zero and selects the active branch.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 0xFFFFFFFF is the documented uint32 MAX. In a preprocessor `#if`
// it must be treated as a non-zero value so the active branch wins.

//TEST:INTERPRET(filecheck=CHECK):

#if 0xFFFFFFFF
static const int picked = 1;
#else
static const int picked = 0;
#endif

void main()
{
    // CHECK: picked=1
    printf("picked=%d\n", picked);
}
