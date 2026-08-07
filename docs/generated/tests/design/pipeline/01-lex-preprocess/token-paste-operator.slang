//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=efc11981208d28591f6638940ff7418ff4c200e6019f91cc90cd83be58370ef5
//META: purpose=The token-paste operator `x##y` synthesizes a fresh single token formed by concatenating the spellings of its two operands.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `x##y` glues two tokens into one identifier. Here
// `MAKE_NAME(my, value)` must paste to `my_value`. If the paste
// operator did not produce a single identifier token, the parser
// would see two tokens or an invalid token and the test would not
// compile.

//TEST:INTERPRET(filecheck=CHECK):

#define PASTE_(a, b) a##_##b
#define PASTE(a, b) PASTE_(a, b)

static const int PASTE(my, value) = 77;

void main()
{
    // CHECK: pasted=77
    printf("pasted=%d\n", my_value);
}
