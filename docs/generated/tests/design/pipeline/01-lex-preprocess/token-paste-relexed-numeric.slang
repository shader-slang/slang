//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=d51763c65e99c32e7704e3dec4e69c5776526cb88e95bd9188137ae60ad362b9
//META: purpose=Pasted tokens are re-lexed from a fresh token-paste source view, so pasting two integer literals forms one new integer literal.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the re-lex step of token pasting with numeric operands:
// concatenating the spellings `12` and `34` and lexing the result
// afresh must yield the single integer literal 1234. If the two
// operand tokens were merely placed adjacent without re-lexing, the
// initializer would be two tokens and would not parse.

//TEST:INTERPRET(filecheck=CHECK):

#define CAT(a, b) a##b

static const int pasted = CAT(12, 34);

void main()
{
    // CHECK: pasted=1234
    printf("pasted=%d\n", pasted);
}
