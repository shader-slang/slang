//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:31Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=377365e730e5a4dfaf556ef64127fd8adbee9dab99b45e23293376d8f95d3072
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
