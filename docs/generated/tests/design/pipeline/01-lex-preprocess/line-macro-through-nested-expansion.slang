//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=efc11981208d28591f6638940ff7418ff4c200e6019f91cc90cd83be58370ef5
//META: purpose=`__LINE__` referenced through nested macro expansion reports the line of the initiating top-level invocation, not a macro-definition line.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the value reported by the __LINE__ builtin derives
// from the initiating top-level location even when it is reached
// through two levels of macro expansion. The use site is line 26 of
// this file; a value of 20 or 21 would mean the builtin was
// attributed to a macro-definition line instead.

//TEST:INTERPRET(filecheck=CHECK):

#define INNER_LINE __LINE__
#define OUTER_LINE INNER_LINE

void main()
{
    // CHECK: line=26
    printf("line=%d\n", OUTER_LINE);
}
