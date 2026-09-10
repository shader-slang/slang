//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#requirecapabilitystmt
//META: doc_section_digest=3fad924642b0316fb757c593b888cc7f5c8b1f201aa0c0264b34655ffafd3fae
//META: purpose=Each name listed by a `__requireCapability` statement is validated as it is read, so a name that does not resolve to a capability is dropped and diagnosed as an unknown capability.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the statement's argument list is checked against the
// capability vocabulary rather than stored blindly. A made-up atom must
// be rejected; the CHECK pins the E36105 unknown-capability diagnostic
// on the offending name.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry computeMain -stage compute

void helper()
{
    __requireCapability(notarealcapability);
//CHECK:                ^^^^^^^^^^^^^^^^^^ unknown capability
//CHECK:                ^^^^^^^^^^^^^^^^^^ unknown capability name 'notarealcapability'.
}

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    helper();
    outputBuffer[0] = 1;
}
