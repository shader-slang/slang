//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#targetswitchstmt-stageswitchstmt-targetcasestmt
//META: doc_section_digest=cd98281cc967041ce075594e739c4a374f2cf6163574c3a2a95a8571934a11b0
//META: purpose=The capability named by a `__target_switch` case label is resolved while the statement is parsed, so a name that is not a known capability is diagnosed as an unknown target name.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the case label is validated as a capability rather than
// treated as an arbitrary identifier. A made-up label must therefore be
// rejected; the CHECK pins the E29110 unknown-target-name diagnostic on
// the label token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry computeMain -stage compute

float helper()
{
    __target_switch
    {
    case notarealtarget:
//CHECK: ^^^^^^^^^^^^^^ unknown target name
//CHECK: ^^^^^^^^^^^^^^ unknown target name 'notarealtarget'
//CHECK: ^^^^^^^^^^^^^^ invalid target_switch case
//CHECK: ^^^^^^^^^^^^^^ 'Invalid' cannot be used as a target_switch case.
        return 1.0;
    default:
        return 9.0;
    }
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = helper();
}
