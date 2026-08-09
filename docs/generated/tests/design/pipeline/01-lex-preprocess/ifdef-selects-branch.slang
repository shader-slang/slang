//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=4608df68ba69a62121bc1827dea7e4379ab2e4bc223340d0aead0d9cc6654fc2
//META: purpose=The preprocessor supports the standard C/HLSL conditional set, including `#ifdef` selecting the active branch when the named macro is defined.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `#ifdef NAME` activates its branch when `NAME` has
// been defined. With `FEATURE` defined, the value 1 is picked; the
// `#else` branch would yield 0.

//TEST:INTERPRET(filecheck=CHECK):

#define FEATURE

#ifdef FEATURE
static const int picked = 1;
#else
static const int picked = 0;
#endif

void main()
{
    // CHECK: picked=1
    printf("picked=%d\n", picked);
}
