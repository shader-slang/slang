//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-10T21:26:16+00:00
//META: source_commit=1ca1aa50e5dbbd12ad65b64a60b3428f23c32d45
//META: doc_ref=docs/generated/design/ir-reference/misc.md#getstringhash
//META: doc_section_digest=93e0a40b474b2be04ad15bdfb24b3f96cd0a29df6e929ab8f254fcc5f2c7154c
//META: purpose=A `getStringHash` argument that does not fold to a string literal is rejected with the documented "must be on a string literal" diagnostic (E41023).
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the restriction that makes the hash meaningful: the operand must
// still be a literal by the time the string-hash check runs, otherwise there
// are no bytes to hash. The argument here is a `String` selected at run time
// from a uniform, so it cannot fold; the CHECK pins the error code rather than
// the message text because the code is the stable part of the contract.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<uint> outputBuffer;
uniform int sel;

[numthreads(1, 1, 1)]
void main()
{
    String s = sel > 0 ? "aaa" : "bbb";
    outputBuffer[0] = getStringHash(s);
//CHECK: E41023
//CHECK:                           ^ span E41023
}
