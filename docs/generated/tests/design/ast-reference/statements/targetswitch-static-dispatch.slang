//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#targetswitchstmt-stageswitchstmt-targetcasestmt
//META: doc_section_digest=bda3b9e2ee202a8291e932d7767da7f7aac9944fa990f3b9e42bb6aa33d4e673
//META: purpose=A TargetSwitchStmt is "a static dispatch chosen at compile time by matching capabilities"; compiling for a target that does not match any TargetCaseStmt selects the `default` body and the unmatched case bodies are resolved away.
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
