//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=3d8d27a59aa70cbff18bfb217be0513b6ff9ff565fb5ebcb7298879f0a47f2fd
//META: purpose=The preprocessor supports the standard C/HLSL `#define`/`#undef` pair, and `#undef` removes a macro so that a subsequent `#ifdef` is false.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Defines a macro, then `#undef`s it. A `#ifdef` after `#undef` must
// be false, so the `#else` branch picks the value 0. If `#undef`
// were a no-op, the `#ifdef` would still be true and the printed
// value would be 1.

//TEST:INTERPRET(filecheck=CHECK):

#define TEMP
#undef TEMP

#ifdef TEMP
static const int picked = 1;
#else
static const int picked = 0;
#endif

void main()
{
    // CHECK: picked=0
    printf("picked=%d\n", picked);
}
