//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=3d8d27a59aa70cbff18bfb217be0513b6ff9ff565fb5ebcb7298879f0a47f2fd
//META: purpose=Boundary probe of `#if`: `2147483647` (int32 MAX) is parsed and evaluated as non-zero.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 2147483647 is signed int32 MAX. The preprocessor must accept this
// literal in a `#if` expression and treat it as truthy.

//TEST:INTERPRET(filecheck=CHECK):

#if 2147483647
static const int picked = 1;
#else
static const int picked = 0;
#endif

void main()
{
    // CHECK: picked=1
    printf("picked=%d\n", picked);
}
