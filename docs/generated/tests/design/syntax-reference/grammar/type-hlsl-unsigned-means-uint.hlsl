//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:02Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#types
//META: doc_section_digest=1b952f18aa4c01c1843e78ee98bfcf640c2d610fbabae24b6d4fd04bdada1ea6
//META: purpose=In HLSL source `unsigned` and `unsigned int` both parse as the `uint` type.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies HLSL-only `unsigned` spellings: the HLSL source stores 3 + 4 via
// `unsigned` and `unsigned int` locals; CHECK asserts the buffer element is
// uint and the folded sum 7U reaches the output.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    unsigned x = 3;
    unsigned int y = 4;
    outputBuffer[0] = x + y;
}

// CHECK: RWStructuredBuffer<uint
// CHECK: 7U
