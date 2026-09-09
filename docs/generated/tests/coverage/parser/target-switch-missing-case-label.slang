//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `__target_switch` body that starts with something other than `case` or `default` is rejected with E20001 naming the two expected keywords.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `parseTargetSwitchStmtImpl` first collects a run of
// `case <capability>:` / `default:` labels, then parses the statements that
// belong to them. If that first loop collects nothing -- because the body
// opens with an ordinary statement instead of a label -- the parser reports
// the token it actually found against the literal expectation
// "'case' or 'default'" and enters recovery.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

[numthreads(1, 1, 1)]
void main()
{
    __target_switch
    {
        int x = 1;
//CHECK:^^^ unexpected token
//CHECK:^^^ unexpected identifier, expected 'case' or 'default'
    }
}
