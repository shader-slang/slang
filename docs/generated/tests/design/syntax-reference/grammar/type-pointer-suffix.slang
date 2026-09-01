//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#types
//META: doc_section_digest=29130252ebb8566d4abf275df716b28c339ea28ecaa9f518d2ddee6bfdd47125
//META: purpose=`CoreType` accepts the pointer suffix `Type '*'` (pointer-to-T via parsePostfixTypeSuffix).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the postfix `*` type spelling used directly in a local
// declaration: `int* p` holds the address of a local, and a write
// through it is observed in the original variable. Runs on the CPU
// target, where pointer types are supported.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int x = 40;
    int* p = &x;
    *p = *p + 2;
    // CHECK: 42
    outputBuffer[0] = x;
}
