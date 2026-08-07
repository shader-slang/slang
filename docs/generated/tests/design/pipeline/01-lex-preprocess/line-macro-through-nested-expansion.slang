//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=377365e730e5a4dfaf556ef64127fd8adbee9dab99b45e23293376d8f95d3072
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
