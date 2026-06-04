//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=1f95bdbd2dae47141e9ce007dbaf0bb66550c8aacddbc7708ffa3f8f5b7ad00d
//META: purpose=`__LINE__` is a constructed token synthesized fresh at each use; it expands to the line number of the use site.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `__LINE__` expands to an integer token at the use
// site. The reference is on line 22 of this file; if the doc claim
// that `__LINE__` is a constructed token attributed to the
// invocation site holds, the printed value must be 22.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: line=22
    printf("line=%d\n", __LINE__);
}
