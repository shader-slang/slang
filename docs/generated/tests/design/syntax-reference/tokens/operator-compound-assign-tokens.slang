//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#operators
//META: doc_section_digest=ed91c7b6d6cd5adc6d847babeaaa96bdb8e0cf1bae7a7fa6718e5d2bdcb70b4f
//META: purpose=Each compound-assignment spelling (`<<=`, `>>=`, `&=`, `\|=`, `^=`, `%=`) lexes as one operator token rather than as an operator followed by a separate `=`.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the maximal-munch lexing of the compound-assignment forms, including
// the three-character shift-assign spellings. The operand is derived from the
// dispatch thread id so the folder cannot pre-compute the chain. Splitting any
// of these into two tokens would either fail to parse or assign a boolean, so
// the single expected buffer value pins all six spellings at once.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x) + 12; // 12
    a <<= 2;                 // 48
    a >>= 1;                 // 24
    a &= 0x1F;               // 24
    a |= 0x40;               // 88
    a ^= 0x03;               // 91
    a %= 100;                // 91
    outputBuffer[0] = a;
    // CHECK: 91
}
