//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=3d8d27a59aa70cbff18bfb217be0513b6ff9ff565fb5ebcb7298879f0a47f2fd
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
