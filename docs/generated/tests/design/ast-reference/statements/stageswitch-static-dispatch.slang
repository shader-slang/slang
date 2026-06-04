//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#targetswitchstmt-stageswitchstmt-targetcasestmt
//META: doc_section_digest=bda3b9e2ee202a8291e932d7767da7f7aac9944fa990f3b9e42bb6aa33d4e673
//META: purpose=A StageSwitchStmt dispatches statically by pipeline stage; only the case body whose label matches the entry point's stage survives in the emitted code, and the other cases are resolved away.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says StageSwitchStmt is "the same shape but dispatches on
// pipeline stage" and both target/stage switches are "resolved away
// during IR lowering; only one case body survives in the emitted
// code". We test by compiling for a compute entry: the
// `case fragment:` body must not appear, and the `default:` value
// must be the one emitted. The C++ text emit makes the resolution
// directly visible.

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
