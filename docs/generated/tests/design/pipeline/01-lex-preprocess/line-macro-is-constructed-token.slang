//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=efc11981208d28591f6638940ff7418ff4c200e6019f91cc90cd83be58370ef5
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
