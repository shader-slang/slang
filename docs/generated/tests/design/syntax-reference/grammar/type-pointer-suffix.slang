//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#types
//META: doc_section_digest=24919d68577e8ac7fef57e804fdf1d69f9063278aeb46ec9c6ca7e3bf3539893
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
