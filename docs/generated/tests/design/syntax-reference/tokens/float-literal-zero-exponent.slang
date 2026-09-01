//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=790d7f70b0af6fa58e0bbfe83b09c796ce08d85a7e54ec041d9c3c7dfdcfd61b
//META: purpose=A bare `0` followed by a decimal exponent (`0e10`) lexes as a single FloatingPointLiteral, not as an integer literal with a bogus suffix.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the leading-zero exponent continuation: `0e10` must be one
// floating-point token. An overload probe distinguishes the outcomes —
// if the literal is floating-point the float overload (kind=1) wins;
// if the exponent were instead swallowed as an integer suffix the file
// would not even compile, and an integer-typed decode would pick
// kind=2. The value probe confirms it decodes to zero.

//TEST:INTERPRET(filecheck=CHECK):

int probe(float x) { return 1; }
int probe(int x) { return 2; }

void main()
{
    // CHECK: kind=1
    printf("kind=%d\n", probe(0e10));
    float v = 0e10;
    // CHECK: v=0
    printf("v=%d\n", int(v));
}
