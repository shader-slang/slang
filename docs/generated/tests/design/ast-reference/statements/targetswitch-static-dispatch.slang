//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#targetswitchstmt-stageswitchstmt-targetcasestmt
//META: doc_section_digest=cd98281cc967041ce075594e739c4a374f2cf6163574c3a2a95a8571934a11b0
//META: purpose=A TargetSwitchStmt is a static dispatch chosen at compile time by matching capabilities; a target that matches no TargetCaseStmt selects the `default` body and the unmatched case bodies do not reach the emitted code.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says each TargetCaseStmt "carries an integer capability
// index and a body. ... Both are resolved away during IR lowering;
// only one case body survives in the emitted code". We compile for
// CPU: the `case hlsl:` body must be eliminated, and the `default:`
// value must be the one emitted into the helper.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -stage compute

float helper()
{
    __target_switch
    {
        case hlsl:
            return 1.0;
        default:
            return 9.0;
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
    //CHECK: return 9.0
}
