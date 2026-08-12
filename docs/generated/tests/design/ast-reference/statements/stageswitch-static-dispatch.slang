//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#targetswitchstmt-stageswitchstmt-targetcasestmt
//META: doc_section_digest=cd98281cc967041ce075594e739c4a374f2cf6163574c3a2a95a8571934a11b0
//META: purpose=A StageSwitchStmt dispatches statically by pipeline stage; only the case body whose label matches the entry point's stage reaches the emitted code.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `__stage_switch` picks its case at compile time from the
// entry point's pipeline stage. A compute entry matches no
// `case fragment:`, so the `default` body must be the emitted one; the
// CHECK asserts the helper's emitted C++ returns the default value.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -stage compute

float helper()
{
    __stage_switch
    {
        case fragment:
            return 1.0;
        default:
            return 0.0;
    }
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=output
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = helper();
    //CHECK: helper
    //CHECK: return 0.0
}
